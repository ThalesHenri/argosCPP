#include "ProcessManager.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <system_error>
#include <unistd.h>
#include <sstream>
#include <csignal>

// Função auxiliar para ler o tempo total de CPU do sistema (/proc/stat)
unsigned long long GetTotalSystemCpuTime() {
    std::ifstream statFile("/proc/stat");
    std::string line;
    if (statFile.is_open() && std::getline(statFile, line)) {
        std::istringstream ss(line);
        std::string cpu;
        ss >> cpu; // Ignora a palavra "cpu"
        
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
        if (ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal) {
            return user + nice + system + idle + iowait + irq + softirq + steal;
        }
    }
    return 0;
}

bool ProcessManager::KillProcess(int pid){
    if (kill(pid, SIGKILL)==0){
        return true;
    }
    return false;
}


void ProcessManager::Update(){
    processes.clear();

    //pega o tamanho da página de memoria do sistema em bytes
    long pageSize = sysconf(_SC_PAGESIZE);

    // Pega o tempo total do sistema atual para o cálculo de delta da CPU
    unsigned long long currentSystemTime = GetTotalSystemCpuTime();

    // Dicionário temporário para o histórico deste frame
    std::unordered_map<int, ProcessCpuTime> currentCpuTimes;

    // Pega a quantidade de cores de CPU para normalizar o ganho (ex: 100% = 1 core totalmente cheio)
    static long numProcessors = sysconf(_SC_NPROCESSORS_ONLN);

    //Itera sobre todos os arquivos dentro de /proc
    for (const auto& entry : std::filesystem::directory_iterator("/proc")){
        if (!entry.is_directory()){
            continue;
        }

        std::string dirName = entry.path().filename().string();

        //Verifica se o nome do diretorio é um PID válido
        if (!std::all_of(dirName.begin(), dirName.end(), ::isdigit)){
            continue;
        }
        ProcessInfo proc;
        proc.pid = std::stoi(dirName);

        //Ler o nome do ProcessInfo
        std::string commPath = "/proc/" + dirName + "/comm";
        std::ifstream commonFile(commPath);
        if(commonFile.is_open()){
            std::getline(commonFile, proc.name);
        }

        // Se o nome estiver em branco (por falta de permissão ou processo fantasma), ignora e pula para o próximo
        if (proc.name.empty()) {
            continue;
        }

        //Ler o uso da memoria
        //O arquivo 'statm' contém várias métricas. O 2º valor é a memória residente
        std::string statmPath = "/proc/" + dirName + "/statm";
        std::ifstream statmFile(statmPath);
        if (statmFile.is_open()){
            long size,resident;
            if (statmFile>>size>>resident){
                //convertendo para bytes e depois para Gigabytes
                proc.memoryGB = (resident*pageSize)/(1024.0*1024.0*1024.0);
            }
        } else {
            proc.memoryGB = 0.0f;
        }
        
        // --- NOVO: CÁLCULO DE USO DE CPU ---
        proc.cpuUsage = 0.0f;
        std::string statPath = "/proc/" + dirName + "/stat";
        std::ifstream statFile(statPath);
        if (statFile.is_open()) {
            std::string statLine;
            if (std::getline(statFile, statLine)) {
                std::istringstream ss(statLine);
                std::string dummy;
                
                // O arquivo /proc/[pid]/stat separa os valores por espaço.
                // O nome do processo está entre parênteses e pode conter espaços (ex: (code server)).
                // Vamos pular os primeiros elementos até fechar o parênteses do nome.
                int pid;
                ss >> pid;
                
                // Pula o campo do nome (pode ocupar mais de um token se tiver espaços)
                std::string nameToken;
                ss >> nameToken;
                while (!nameToken.empty() && nameToken.back() != ')') {
                    ss >> nameToken;
                }
                
                char state;
                int ppid, pgrp, session, tty_nr, tpgid;
                unsigned int flags;
                unsigned long minflt, cminflt, majflt, cmajflt;
                unsigned long long utime = 0, stime = 0;
                
                // Lê os próximos tokens até chegar no 14º e 15º (utime e stime)
                ss >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags 
                   >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime;

                float rawCpuUsage = 0.0f;

                // Se já tínhamos esse processo no frame anterior, calculamos a diferença (delta)
                if (lastCpuTimes.find(proc.pid) != lastCpuTimes.end()) {
                    unsigned long long oldProcessTime = lastCpuTimes[proc.pid].utime + lastCpuTimes[proc.pid].stime;
                    unsigned long long newProcessTime = utime + stime;
                    unsigned long long processDelta = newProcessTime - oldProcessTime;

                    unsigned long long systemDelta = currentSystemTime - lastCpuTimes[proc.pid].totalSystemTime;

                    if (systemDelta > 0) {
                        // Multiplicamos pelo número de processadores para refletir a escala real (0% a 100%)
                        rawCpuUsage = (static_cast<float>(processDelta) / systemDelta) * 100.0f * numProcessors;
                        
                        // Limitador de segurança para evitar picos estranhos do kernel ao iniciar threads
                        if (rawCpuUsage > 100.0f * numProcessors) rawCpuUsage = 100.0f * numProcessors;

                        // Aplica a Média Móvel Exponencial (EMA) para suavizar a flutuação (30% atual, 70% histórico)
                        proc.cpuUsage = (rawCpuUsage * 0.3f) + (lastCpuTimes[proc.pid].lastCpuUsage * 0.7f);
                    }
                } else {
                    // Se é a primeira vez que vemos o processo, o uso bruto inicial é a nossa base
                    proc.cpuUsage = rawCpuUsage;
                }

                // Salva os valores atuais no mapa temporário (incluindo o uso de CPU suavizado deste frame)
                ProcessCpuTime cpuTime = { utime, stime, currentSystemTime, proc.cpuUsage };
                currentCpuTimes[proc.pid] = cpuTime;
            }
        }

        processes.push_back(proc);
    }  

    // Atualiza o histórico global com os dados coletados neste frame
    lastCpuTimes = std::move(currentCpuTimes);

    // Ordena o vetor final pelo uso de memória de forma decrescente (maior para o menor)
    std::sort(processes.begin(), processes.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
        return a.memoryGB > b.memoryGB;
    });
}

const std::vector<ProcessInfo>& ProcessManager::GetProcesses() const {
    return processes;
}

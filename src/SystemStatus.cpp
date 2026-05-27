#include "SystemStatus.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>


SystemStatus::SystemStatus()
    : prevTotalTicks(0),prevIdleTicks(0),currentCpuUsage(0.0),totalRamGB(0.0),usedRamGB(0.0),ramPercent(0.0)
{
    cpuHistory.resize(90,0.0); // Histórico de 90 pontos para o Gráfico
    Update();
}
SystemStatus::~SystemStatus(){

}

void SystemStatus::readCPU(){
    FILE* file = fopen("/proc/stat", "r");
    if (!file)return;

    char cpu[8];
    long user,nice,system,idle,iowait,irq,softirq,steal;
    if (fscanf(file, "%s %ld %ld %ld %ld %ld %ld %ld %ld", cpu, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) == 9) {

        long idleTicks = idle + iowait;
        long activeTicks = user + nice + system + irq + softirq + steal;
        long totalTicks = idleTicks + activeTicks;


        long totalDiff = totalTicks - prevTotalTicks;
        long idleDiff = idleTicks - prevIdleTicks;

        if (totalDiff > 0){
            currentCpuUsage = (float)(totalDiff-idleDiff)/totalDiff * 100.0f;

            //Move o histórico para a esquerda e adiciona o dado novo no fim
            for (size_t i = 0; i< cpuHistory.size() - 1; ++i){
                cpuHistory[i] = cpuHistory[i + 1];
            }
            cpuHistory[cpuHistory.size() - 1] = currentCpuUsage;
        }
        prevTotalTicks = totalTicks;
        prevIdleTicks = idleTicks;
    }
    fclose(file);
    
}

void SystemStatus::readRAM(){
    FILE* file = fopen("/proc/meminfo", "r");
    if (!file) return;

    char line[128];
    long memTotal = 0;
    long memAvailable = 0;
    long memFree = 0; // Fallback caso o kernel não use MemAvailable

    // Lendo o arquivo linha por linha de forma segura
    while (fgets(line, sizeof(line), file)) {
        long value = 0;
        if (sscanf(line, "MemTotal: %ld kB", &value) == 1) {
            memTotal = value;
        } else if (sscanf(line, "MemAvailable: %ld kB", &value) == 1) {
            memAvailable = value;
        } else if (sscanf(line, "MemFree: %ld kB", &value) == 1) {
            memFree = value;
        }
    }
    fclose(file);

    // Se o sistema não tiver MemAvailable, usa o MemFree como plano B
    if (memAvailable == 0) {
        memAvailable = memFree;
    }

    if (memTotal > 0 && memAvailable > 0) {
        long memUsed = memTotal - memAvailable;
        
        // Conversão exata para GB
        totalRamGB = (float)memTotal / (1024.0f * 1024.0f);
        usedRamGB = (float)memUsed / (1024.0f * 1024.0f);
        ramPercent = (float)memUsed / (float)memTotal;
    } else {
        // Valores de segurança caso a leitura falhe por completo
        totalRamGB = 0.0f;
        usedRamGB = 0.0f;
        ramPercent = 0.0f;
    }
}
void SystemStatus::Update(){
    readCPU();
    readRAM();
}

#pragma once
#include <string>
#include <unordered_map>
#include <vector>

//Estrutura para armazenar os dados de cada processo

struct ProcessInfo{
    int pid;
    std::string name;
    float cpuUsage;
    float memoryGB;

};


struct ProcessCpuTime{
    unsigned long long utime;
    unsigned long long stime;
    unsigned long long totalSystemTime;
    float lastCpuUsage;
};

class ProcessManager{
    public:
        void Update(); //Fara a leitura do /proc 
        const std::vector<ProcessInfo> &GetProcesses() const;
        bool KillProcess(int pid);

    private:
        std::vector<ProcessInfo> processes;
        std::unordered_map<int,ProcessCpuTime> lastCpuTimes;
};

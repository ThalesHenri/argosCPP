#pragma once
#include <vector>
#include <string>

class SystemStatus{
    public:
        
        //Construtor
        SystemStatus();
        //Destrutor
        ~SystemStatus();
        //Getters para a UI
        float GetCPUUsage() const { return currentCpuUsage; }
        const std::vector<float>& GetCPUHistory() const { return cpuHistory; }

        float GetTotalRAM() const {return totalRamGB; }
        float GetUsedRAM() const {return usedRamGB; }
        float GetRAMPercent() const {return ramPercent; }
        //Atualiza os dados(vai ser chamado a cada frame)
        void Update();

    private:
        //Cálculos da CPU
        long prevTotalTicks;
        long prevIdleTicks;
        float currentCpuUsage;
        std::vector<float> cpuHistory;

        //Variáveis de Ram
        float totalRamGB;
        float usedRamGB;
        float ramPercent;

        //Métodos internos
        void readCPU();
        void readRAM();

};

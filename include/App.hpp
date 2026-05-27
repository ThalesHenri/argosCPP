#pragma once

//importando as dependencias
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "ProcessManager.hpp"

class App {
    public: //métodos públicos
        App(); //Construtor
        ~App(); //Destrutor: limpa a memória 

        void Run();

    private: //métodos privados
        
        bool initWindow();
        bool initImGui();
        void renderUI();
        float getCPUUsage();

        // atributos
        GLFWwindow* window;
        std::vector<float> cpuHistory;
        bool running;
        GLuint iconTexture = 0;
        int iconWidth = 0;
        int iconHeight = 0;
        ProcessManager procManager;
};


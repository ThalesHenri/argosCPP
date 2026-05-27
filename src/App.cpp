#include "App.hpp"
#include <GL/gl.h>
#include <ostream>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "SystemStatus.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <cstdint>
#include <cstdio>
#include <iostream>

//inicializando SystemStatus
SystemStatus stats;

App::App() : window(nullptr),running(true){
    if (!initWindow() || !initImGui()){

        running = false;
    }
}

App::~App(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}

bool App::initWindow() {
    if (!glfwInit()) {
        return false;
    }

    window = glfwCreateWindow(1280, 720, "Argos- Gerenciador de Recurso", NULL, NULL);
    if (!window) {
        return false;
    }

    // ─── CRUCIAL PARA O OPENGL FUNCIONAR ─────────────────────────────
    // Ativa o contexto na thread atual e inicializa os ponteiros do GLEW
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "[Argos] Erro ao inicializar o GLEW!" << std::endl;
        return false;
    }
    // ─────────────────────────────────────────────────────────────────
    
    //Evita o serrilhado do icone 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // --- CARREGAMENTO ÚNICO DA IMAGEM (JANELA + GUI) ---
    int width, height, channels;
    
    
    unsigned char* pixels = stbi_load("assets/ArgosIcon2.png", &width, &height, &channels, 4); 
    
    if (pixels) {
        std::cout << "[Argos] Imagem carregada com sucesso! Dimensões: " << width << "X" << height << std::endl;

        // 1. Configura o ÍCONE DA JANELA do GLFW
        GLFWimage windowIcon[1];
        windowIcon[0].width = width;
        windowIcon[0].height = height;
        windowIcon[0].pixels = pixels;
        glfwSetWindowIcon(window, 1, windowIcon);

        // 2. Configura a TEXTURA OPENGL para a GUI do ImGui
        iconWidth = width;
        iconHeight = height;
        
        glGenTextures(1, &iconTexture);
        glBindTexture(GL_TEXTURE_2D, iconTexture);

        //Evita falha de pixel
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // Filtros para a imagem ficar nítida
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Envia os dados para a GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0); // Desvincula de forma limpa

        // 3. Deleta a cópia da RAM pois ela já reside na GPU
        stbi_image_free(pixels);
        
        std::cout << "[Argos] Textura OpenGL criada com sucesso para a GUI!" << std::endl;
    } else {
        std::cerr << "[Argos] Erro crônico ao carregar a imagem!" << std::endl;
        std::cerr << "Motivo do STB: " << stbi_failure_reason() << std::endl;
    }

    return true;
}

bool App::initImGui(){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
    return true;
}

void App::renderUI(){
    // 1. Definição do Tamanho da Janela
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)width, (float)height), ImGuiCond_Always);

    // 2. Flags da Janela
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | 
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | 
                             ImGuiWindowFlags_NoCollapse;

    // CORREÇÃO: O Begin DEVE vir antes de qualquer desenho (como o logo)!
    ImGui::Begin("Dashboard", nullptr, flags);

    // --- DESENHANDO A LOGO NO LUGAR CORRETO ---
    if (iconTexture != 0){
        // Mantém a proporção exata da sua imagem (1332x800) para não esmagar o desenho
        float alturaDesejada = 40.0f; 
        float larguraProporcional = alturaDesejada * (1332.0f / 800.0f); // Dá ~66.6f
        
        ImVec2 logoSize(larguraProporcional, alturaDesejada);
        ImGui::GetWindowDrawList()->AddImage(
                (void*)(uintptr_t)iconTexture,
                ImGui::GetCursorScreenPos(),
                ImVec2(ImGui::GetCursorScreenPos().x + logoSize.x,ImGui::GetCursorScreenPos().y + logoSize.y),
                ImVec2(0,0),ImVec2(1,1),
                ImColor(255,255,255,255)
                );
        ImGui::Dummy(logoSize);
        
        // Alinha verticalmente o texto com o centro da imagem
        ImGui::SameLine(0.0f, 12.0f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (alturaDesejada / 4.0f)); 
    }

    // Título alinhado ao lado da logo devido ao SameLine()
    ImGui::Text("Monitoramento em Tempo Real");
    ImGui::Spacing();

    // --- GRÁFICO DA CPU ---
    ImGui::Text("Uso da CPU: %.1f%%", stats.GetCPUUsage());
    ImGui::PlotLines("##CPU_Graph", stats.GetCPUHistory().data(), stats.GetCPUHistory().size(), 0, nullptr, 0.0f, 100.0f, ImVec2(-1, 80));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- BARRA DA MEMÓRIA RAM ---
    ImGui::Text("Memória RAM: %.2f GB / %.2f GB", stats.GetUsedRAM(), stats.GetTotalRAM());
    
    char ramPercentStr[32];
    sprintf(ramPercentStr, "%.1f%%", stats.GetRAMPercent() * 100.0f);
    ImGui::ProgressBar(stats.GetRAMPercent(), ImVec2(-1, 0), ramPercentStr);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ---  TABELA DE PROCESSOS ---
    ImGui::Text("Processos em Execução");
    ImGui::Spacing();

    // Aqui vai entrar o loop da tabela usando ImGui::BeginTable
    // Deixando o espaço reservado para quando você criar o ProcessManager
    if (ImGui::BeginTable("##Processos", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, -1))) {
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Nome", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("CPU %", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Memória", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableHeadersRow();

        // Loop que desenha uma linha para cada processo lido
        for (const auto& proc : procManager.GetProcesses()) {
            ImGui::TableNextRow();
            
            //PID
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", proc.pid);
            
            //Nome
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", proc.name.c_str());
            
            //CPU
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.1f%%", proc.cpuUsage);
            
            //Memoria
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.4f GB", proc.memoryGB);
            // --- NOVA COLUNA: AÇÃO DE KILL ---
            ImGui::TableSetColumnIndex(4);
    
            // Usamos um ID único para cada botão usando o PID do processo
            std::string buttonLabel = "Kill##" + std::to_string(proc.pid);
    
            // Estiliza o botão com uma cor vermelha
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));

            // CALCULO DE CENTRALIZAÇÃO:
            // Pega a largura total disponível na coluna atual da tabela
            float columnWidth = ImGui::GetContentRegionAvail().x;
            // Define a largura fixa que você quer para o botão (ex: 50 pixels)
            float buttonWidth = 50.0f; 
    
            // Move o cursor para a direita para centralizar o botão na coluna
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - buttonWidth) * 0.5f);

            // Passamos o tamanho fixo (width, height) no segundo argumento. 0.0f de altura usa o padrão.
            if (ImGui::Button(buttonLabel.c_str(), ImVec2(buttonWidth, 0.0f))) {
                if (procManager.KillProcess(proc.pid)) {
                    procManager.Update(); 
                } else {
                    std::cerr << "Não foi possível encerrar o processo " << proc.pid << std::endl;
                }
            }
    
            ImGui::PopStyleColor(3); // Restaura as cores padrão
        }
        ImGui::EndTable();
    }

    ImGui::End(); // Fecha a janela "Dashboard" de forma limpa
}
void App::Run() {
    double lastTime = glfwGetTime();//tempo em segundos
                                   //
                    
    while (running && !glfwWindowShouldClose(window)) {
        glfwPollEvents();
        double currentTime = glfwGetTime();
        if (currentTime - lastTime >= 0.1){
            stats.Update();
            procManager.Update();
            lastTime = currentTime;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderUI();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

}


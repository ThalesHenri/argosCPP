📦 Argos v1.0.0 — Release Notes

Bem-vindo ao lançamento oficial do Argos (v1.0.0)! Esta é a primeira versão estável do nosso monitor de recursos e gerenciador de processos de baixo nível para Linux, construído inteiramente em C++ com uma interface gráfica rica via Dear ImGui e OpenGL.
💡 A Motivação: O "Gerenciador do Windows" no Linux

O Argos surgiu de uma necessidade muito clara: trazer a praticidade visual e o controle direto do Gerenciador de Tarefas do Windows para o ecossistema Linux.

Embora o Linux seja incrivelmente poderoso e ofereça ferramentas fantásticas via terminal (como top e htop), muitos desenvolvedores e usuários sentem falta de uma interface de janela dedicada, puramente gráfica, que combine:

    Visualização Centralizada: Uma tabela limpa onde você bate o olho e sabe exatamente quem está devorando a sua memória RAM ou ciclando a sua CPU.

    Ação Sem Fricção: A capacidade de encerrar uma aplicação travada com um único clique do mouse, sem precisar abrir o terminal, descobrir o PID manualmente e digitar comandos de kill.

O Argos foi desenhado para preencher essa lacuna, unindo a performance nativa do C++ com a agilidade de clique que torna o fluxo de trabalho muito mais fluido.
🚀 O que há de novo?
🧠 Core & Processamento de Dados do Kernel

    Varredura Dinâmica do /proc: Implementação de varredura automatizada no sistema de arquivos virtuais do Linux para mapear PIDs ativos, consumos de memória residente e tempos de CPU de forma assíncrona.

    Leitura Avançada de Metadados (cmdline): Adicionado suporte para leitura através do /proc/[pid]/cmdline, contornando o limite rígido de 15 caracteres do kernel e exibindo os nomes reais e completos de subprocessos.

📉 Algoritmos & Suavização Visual

    Média Móvel Exponencial (EMA): Implementação de um algoritmo de amortecimento matemático (30% peso atual, 70% peso histórico) aplicado ao cálculo de delta de tempo da CPU para eliminar comportamentos instáveis e picos abruptos.

    Otimização de Frequência de Amostragem: Ajuste do timer de renderização global no loop principal da aplicação (GLFW) para atualizar a cada 1.5s, reduzindo drasticamente o overhead de I/O do próprio monitor na máquina hospedeira.

🛠️ Gerenciamento Ativo (Recursos de Interface)

    Botão Kill Integrado: Adicionada uma coluna de ação com botões compactos e centralizados em tom vermelho. O botão realiza chamadas de sistema nativas enviando um sinal SIGKILL para derrubar processos imediatamente.

    Ordenação por RAM: Sistema de ordenação automática decrescente baseado no consumo de memória convertido precisamente de páginas de sistema para Gigabytes (GB).

📦 Detalhes do Pacote de Distribuição

O Argos agora conta com uma estrutura compacta e simplificada de empacotamento na pasta release/ para rodar em qualquer distribuição Linux x86_64:
Plaintext

release/
├── argos                 (Binário otimizado compilado com GCC -O3)
├── argos-launcher.sh     (Script bash utilitário de ambiente)
└── assets/               
    └── icon.png          (Identidade visual do software)

    Launcher Script: O script utilitário agora aponta internamente para a pasta raiz release/, resolvendo os caminhos relativos de ativos mesmo quando invocado de fora do diretório.

    Suporte a Atalho do Sistema: O arquivo descritor argos.desktop foi atualizado para mapear o executável final dentro da nova estrutura.

🔧 Correções de Bugs (Bugfixes)

    Idêntico ao Gerenciador de Tarefas: Corrigida a escala de leitura de CPU para respeitar o limite global de 100% da máquina, eliminando a inflação artificial causada pelo número de processadores.

    Tratamento de Tokens com Espaço: Corrigido o parser do /proc/[pid]/stat que quebrava o alinhamento de tokens ao ler processos cujos nomes continham espaços entre parênteses.

    Tratamento de Strings Nulas: Blindagem na busca de caracteres nulos na leitura de comandos do sistema, evitando falhas de segmentação em subprocessos.

🤝 Faça parte do Argos! (Contribua)

O Argos está apenas no começo e o código é 100% open-source! Se você achou a ideia massa e quer dar um tapa no projeto, toda ajuda é super bem-vinda:

    📊 Gráficos de Linha: Criar gráficos em tempo real para o uso histórico global de CPU e RAM.

    🔍 Barra de Pesquisa: Filtrar a tabela de processos digitando o nome ou o PID.

    🌗 Temas: Adicionar um alternador de tema claro/escuro na interface do ImGui.

Como começar?

    Faça um Fork do repositório.

    Crie uma branch para a sua feature (git checkout -b feature/minha-melhoria).

    Abra um Pull Request detalhando o que você alterou.

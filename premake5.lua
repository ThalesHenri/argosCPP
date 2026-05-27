workspace "ArgosProject"
    configurations { "Debug", "Release" }
    architecture "x64"

project "argos"
    kind "ConsoleApp" -- Use "WindowedApp" se quiser esconder o terminal no Windows futuramente
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "bin-int/%{cfg.buildcfg}"

    files {
        "src/**.h",
        "src/**.cpp",
        -- Incluindo o ImGui explicitamente
        "external/imgui/*.h",
        "external/imgui/*.cpp",
        "external/imgui/backends/imgui_impl_glfw.cpp",
        "external/imgui/backends/imgui_impl_opengl3.cpp"
    }

    includedirs {
        "include",
        "external/imgui",
        "external/imgui/backends",
        "/usr/include",       -- Caminho padrão Linux
        "/usr/local/include"
    }

    -- Bibliotecas para linkar (Linux)
    links { "GL", "glfw", "GLEW", "pthread", "dl" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

#pragma once

#include <filesystem>

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

namespace rasterizer::ui {
    inline void initialize(SDL_Window* window, SDL_Renderer* renderer) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = "../bin/imgui.ini";
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
        ImGui::StyleColorsDark();
        const std::string alexandriaFontPath = "../assets/font/Alexandria.ttf";
        if (std::filesystem::exists(std::filesystem::path(alexandriaFontPath))) {
            const char* rawFontPath = alexandriaFontPath.c_str();
            rasterizer::print("Loading font into ImGui: %s\n", rawFontPath);
            io.Fonts->AddFontFromFileTTF(rawFontPath, 24.0f);
        }
        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);
    }

    inline bool processInput(const SDL_Event* event) {
        ImGui_ImplSDL2_ProcessEvent(event);
        const auto& imGuiIo = ImGui::GetIO();
        return imGuiIo.WantCaptureMouse || imGuiIo.WantCaptureKeyboard;
    }

    void newFrame() {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void render() {
        bool showDemoWindow = true;
        ImGui::ShowDemoWindow(&showDemoWindow);

        ImGui::Render();
    }

    void present(SDL_Renderer* renderer) {
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    }

    void destroy() {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }
}

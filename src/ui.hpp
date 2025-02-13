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
        const std::string alexandriaFontPath = "../assets/font/JetBrainsMono-Regular.ttf";
        if (std::filesystem::exists(std::filesystem::path(alexandriaFontPath))) {
            // Build font atlas with basic range + arrows
            static constexpr std::array<ImWchar, 5> ranges {
                0x0020, 0x00FF,  // Basic Latin
                0x2190, 0x2193,  // Arrows
                0,
            };
            const char* rawFontPath = alexandriaFontPath.c_str();
            rasterizer::print("Loading font into ImGui: {}\n", rawFontPath);
            io.Fonts->AddFontFromFileTTF(rawFontPath, 20.0f, nullptr, ranges.data());
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

    void render(glm::vec3 frustumEye, glm::vec3 frustumForward, bool backfaceCullingEnabled,
                bool isPointModeEnabled, bool isLineModeEnabled, bool isFillModeEnabled,
                std::int32_t fillModeIndex, std::int32_t rasterizationRuleIndex) {
        // Set the position to (16, 16) from the top-left
        constexpr ImVec2 windowPos(16.0f, 16.0f);
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

        ImGui::SetWindowFocus(nullptr);
        ImGui::Begin("Rasterizer", nullptr,
                     ImGuiWindowFlags_NoNavFocus |
                     ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoBringToFrontOnFocus |
                     ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::SeparatorText("Frustum");
        ImGui::InputFloat3(" Eye", glm::value_ptr(frustumEye));
        ImGui::InputFloat3(" Forward", glm::value_ptr(frustumForward));

        ImGui::SeparatorText("Rendering");
        ImGui::Checkbox("Backface Culling ", &backfaceCullingEnabled);
        static constexpr std::array fillModeLabels{"Vertex (Random)", "Texture"};
        static constexpr std::array rasterizationRuleLabels{"DDA", "Top-Left"};
        ImGui::Checkbox("Point ", &isPointModeEnabled);
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(20.0f, 0.0f));
        ImGui::SameLine();
        ImGui::Checkbox("Line ", &isLineModeEnabled);
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(20.0f, 0.0f));
        ImGui::SameLine();
        ImGui::Checkbox("Fill ", &isFillModeEnabled);
        ImGui::BeginDisabled(!isFillModeEnabled);
        ImGui::Combo(" Fill Mode", &fillModeIndex, fillModeLabels.data(), fillModeLabels.size());
        ImGui::Combo(" Rasterization Rule",
                     &rasterizationRuleIndex, rasterizationRuleLabels.data(), rasterizationRuleLabels.size());
        ImGui::EndDisabled();

        ImGui::SeparatorText("Controls");
        ImGui::Columns(2, "Controls Table", true);
        ImGui::SetColumnWidth(0, 144.0f);
        // Rendering parameters
        ImGui::Text("1 - 6");
        ImGui::NextColumn();
        ImGui::Text("Polygon mode configurations");
        ImGui::NextColumn();
        // Frustum eye movement
        ImGui::Text("W / A / S / D");
        ImGui::NextColumn();
        ImGui::Text("Move frustum eye around");
        ImGui::NextColumn();
        // Frustum forward rotation
        ImGui::Text("↑ / ↓ / ← / →");
        ImGui::NextColumn();
        ImGui::Text("Rotate frustum forward direction");
        ImGui::NextColumn();
        // Backface culling
        ImGui::Text("C");
        ImGui::NextColumn();
        ImGui::Text("Toggle backface culling");
        ImGui::NextColumn();
        // Rasterization rule
        ImGui::Text("X / Z");
        ImGui::NextColumn();
        ImGui::Text("DDA / Top-Left rasterization");
        ImGui::NextColumn();
        // Esc
        ImGui::Text("Esc");
        ImGui::NextColumn();
        ImGui::Text("Close the application");
        ImGui::NextColumn();
        // Set back to single column
        ImGui::Columns(1);

        ImGui::End();

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

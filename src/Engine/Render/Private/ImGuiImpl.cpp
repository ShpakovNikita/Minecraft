#include "Engine/Render/ImGuiImpl.hpp"

#define IMGUI_IMPL_METAL_CPP 1

#include <imgui.h>
#include <imgui_impl_opengl3.h>
// Forked backend impl with some tweaks
#include <Engine/Render/Metal/ImGui/imgui_impl_metal.h>
#include <imgui_impl_sdl.h>

#include "Engine/Core/Window.hpp"

#include "Engine/Render/Metal/Renderer.hpp"
#include "Engine/Render/Metal/LogicalDevice.hpp"
#include "Engine/Render/OpenGl/WindowContext.hpp"
#include "Engine/Render/Metal/WindowContext.hpp"
#include "Engine/Render/RenderContext.hpp"
#include "Engine/Render/Renderer.hpp"

using namespace SHV;

namespace SImGui {
void SetUpMetalBackend(RenderContext& renderContext) {
    auto& renderer = static_cast<Metal::Renderer&>(renderContext.GetRenderer());

    ImGui_ImplMetal_Init(&renderer.GetLogicalDevice().GetDevice());
    ImGui_ImplSDL2_InitForMetal(&renderContext.GetWindow().GetWindow());
}

void SetUpOpenGlBackend(RenderContext& renderContext) {
    auto& windowContext =
        static_cast<OpenGl::WindowContext&>(renderContext.GetWindowContext());

    ImGui_ImplSDL2_InitForOpenGL(&renderContext.GetWindow().GetWindow(),
                                 windowContext.GetGlContext());
    ImGui_ImplOpenGL3_Init("#version 330");
}

void TearDownMetalBackend() {
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplSDL2_Shutdown();
}

void TearDownOpenGlBackend() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
}

void BeginFrameMetal(RenderContext& renderContext) {
    auto& renderer = static_cast<Metal::Renderer&>(renderContext.GetRenderer());

    ImGui_ImplMetal_NewFrame(&renderer.GetRenderPassDescriptor());
}

void BeginFrameOpenGl() { ImGui_ImplOpenGL3_NewFrame(); }

void EndFrameMetal(RenderContext& renderContext) {
    auto& renderer = static_cast<Metal::Renderer&>(renderContext.GetRenderer());

    ImGui_ImplMetal_RenderDrawData(::ImGui::GetDrawData(),
                                   &renderer.GetCommandBuffer(),
                                   &renderer.GetRenderCommandEncoder());
}

void EndFrameOpenGl() {
    ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());
}
}  // namespace SImGui

SHV::ImGui::ImGui(RenderContext& aRenderContext)
    : renderContext(aRenderContext) {}

void SHV::ImGui::SetUp() {
    ::ImGui::DebugCheckVersionAndDataLayout(
        IMGUI_VERSION, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2),
        sizeof(ImVec4), sizeof(ImDrawVert), sizeof(ImDrawIdx));
    ::ImGui::CreateContext();

    ImGuiStyle style;
    style.WindowBorderSize = 0.0f;
    style.WindowRounding = 12.0f;
    ::ImGui::StyleColorsDark(&style);

    switch (renderContext.GetRenderApi()) {
        case eRenderApi::kOpenGLES:
            SImGui::SetUpOpenGlBackend(renderContext);
            break;
        case eRenderApi::kMetal:
            SImGui::SetUpMetalBackend(renderContext);
            break;
    }
};

void SHV::ImGui::TearDown() {
    switch (renderContext.GetRenderApi()) {
        case eRenderApi::kOpenGLES:
            SImGui::TearDownOpenGlBackend();
            break;
        case eRenderApi::kMetal:
            SImGui::TearDownMetalBackend();
            break;
    }

    ::ImGui::DestroyContext();
};

void SHV::ImGui::BeginFrame() {
    switch (renderContext.GetRenderApi()) {
        case eRenderApi::kOpenGLES:
            SImGui::BeginFrameOpenGl();
            break;
        case eRenderApi::kMetal:
            SImGui::BeginFrameMetal(renderContext);
            break;
    }
    ImGui_ImplSDL2_NewFrame();

    ::ImGui::NewFrame();
}

void SHV::ImGui::EndFrame() {
    ::ImGui::Render();
    switch (renderContext.GetRenderApi()) {
        case eRenderApi::kOpenGLES:
            SImGui::EndFrameOpenGl();
            break;
        case eRenderApi::kMetal:
            SImGui::EndFrameMetal(renderContext);
            break;
    }
}

void SHV::ImGui::PollEvents(SDL_Event* event) {
    ImGui_ImplSDL2_ProcessEvent(event);
}
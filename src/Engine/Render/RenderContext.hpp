#pragma once

#include <memory>

#include "Renderer.hpp"
#include "WindowConfig.hpp"

namespace SHV {

class Window;
class WindowContext;
class ImGui;

/**
 * This instance manages render instances based on selected API option
 */
class RenderContext {
   public:
    RenderContext(const WindowConfig& windowConfig,
                  const eRenderApi& renderApi);
    ~RenderContext();

    void SetUp();
    void TearDown();

    Renderer& GetRenderer() const;
    Window& GetWindow() const;
    ImGui& GetImGui() const;
    WindowContext& GetWindowContext() const;

    void SwitchRenderApi(const eRenderApi& aRenderApi);
    const eRenderApi& GetRenderApi() const;

   private:
    void CreateMetalApiContext();
    void CreateOpenGlContext();

    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<WindowContext> windowContext;
    std::unique_ptr<Window> window;
    std::unique_ptr<ImGui> imGui;

    const WindowConfig windowConfig;
    eRenderApi renderApi;
};
}  // namespace SHV
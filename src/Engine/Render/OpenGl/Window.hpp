#pragma once

#include "Engine/Render/Window.hpp"

using SDL_GLContext = void*;

namespace SHV {
namespace OpenGl {
class Window : public ::SHV::Window {
   public:
    Window(const WindowConfig& config);

    SDL_GLContext GetGlContext();

    virtual const glm::vec2 GetViewportSize() const;

   private:
    virtual void OnSetUpComplete();
    virtual void OnTearDownBegin();

    SDL_GLContext context;
};
}  // namespace OpenGl
}  // namespace SHV
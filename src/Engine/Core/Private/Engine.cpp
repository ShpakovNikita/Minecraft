#include "Engine/Core/Engine.hpp"

#include <SDL.h>

#include <chrono>
#include <thread>

#include <imgui.h>

#include "Engine/Common/Exception.hpp"
#include "Engine/Common/Logger.hpp"
#include "Engine/Core/ImmutableConfig.hpp"
#include "Engine/Core/MutableConfig.hpp"
#include "Engine/Render/ImGuiImpl.hpp"
#include "Engine/Render/RenderContext.hpp"
#include "Engine/Render/Model/Material.hpp"
#include "Engine/Render/Model/Primitive.hpp"
#include "Engine/Tools/Toolbar.hpp"
#include "Engine/ECS/Scene.hpp"
#include "Engine/ECS/Components/RenderComponent.hpp"
#include "Engine/ECS/Components/TransformComponent.hpp"

// TODO: remove
#include "Engine/Render/OpenGl/ECS/Systems/RenderSystem.hpp"

#include <entt/entity/registry.hpp>

using namespace SHV;

Engine::Engine(const ImmutableConfig& aImmutableConfig,
               MutableConfig& aMutableConfig)
    : immutableConfig(aImmutableConfig),
      mutableConfig(aMutableConfig),
      renderContext(nullptr) {}

int Engine::Run() noexcept {
    LogI(eTag::kEngine) << "Engine setup begin" << std::endl;
    try {
        SetUp();
    } catch (SHV::Exception& e) {
        SHV::LogE(SHV::eTag::kBase)
            << "Critical error occurred during engine SetUp: " << e.what();
        return EXIT_FAILURE;
    }
    LogI(eTag::kEngine) << "Engine setup complete" << std::endl;

    // Executes until some command to close engine
    MainLoop();

    LogI(eTag::kEngine) << "Engine tear down begin" << std::endl;
    try {
        TearDown();
    } catch (SHV::Exception& e) {
        SHV::LogE(SHV::eTag::kBase)
            << "Critical error occurred during engine TearDown: " << e.what();
        return EXIT_FAILURE;
    }
    LogI(eTag::kEngine) << "Engine tear down complete" << std::endl;

    return EXIT_SUCCESS;
}

void Engine::SetUp() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw Exception("SDL could not initialize! SDL_Error: %s\n",
                        SDL_GetError());
    }

    const WindowConfig windowConfig = {
        immutableConfig.width, immutableConfig.height,
        immutableConfig.windowName, immutableConfig.renderApi};
    renderContext = std::make_unique<RenderContext>(windowConfig,
                                                    immutableConfig.renderApi);
    renderContext->SetUp();

    imgui = std::make_unique<ImGui>(*renderContext);
    imgui->SetUp();

    scene = std::make_unique<Scene>();
    scene->AddSystem<OpenGl::RenderSystem>();
    scene->SetUp();

    toolbar = std::make_unique<Toolbar>(*scene);

    LoadPrimitives();
}

void Engine::TearDown() {
    UnloadPrimitives();

    scene->TearDown();
    scene = nullptr;

    toolbar = nullptr;

    imgui->TearDown();
    imgui = nullptr;

    renderContext->TearDown();
    renderContext = nullptr;

    SDL_Quit();
}

void Engine::MainLoop() {
    isRunning = true;
    const float minSecPerFrame = 1.0f / mutableConfig.fpsLimit;

    LogI(eTag::kEngine) << "Engine starting main loop" << std::endl;

    auto previousTime = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::microseconds(
        static_cast<size_t>(minSecPerFrame * 1000.0f)));

    while (isRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = currentTime - previousTime;
        float deltaTime =
            std::chrono::duration_cast<std::chrono::nanoseconds>(elapsedTime)
                .count() /
            (1000.0f * 1000.0f * 1000.0f);

        Tick(deltaTime);

        std::chrono::milliseconds timeToSleep(std::max(
            0, static_cast<int>((minSecPerFrame - deltaTime) * 1000.0f)));
        std::this_thread::sleep_for(timeToSleep);

        previousTime = currentTime;
    }

    LogI(eTag::kEngine) << "Engine ended main loop" << std::endl;
}

void Engine::Tick(float deltaTime) {
    PollEvents(deltaTime);
    UpdateSystems(deltaTime);
    RenderLoop(deltaTime);
}

void Engine::PollEvents(float /* deltaTime */) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        imgui->PollEvents(&e);
        if (e.type == SDL_QUIT) isRunning = false;
    }
}

void Engine::UpdateSystems(float deltaTime) { scene->Process(deltaTime); }

void Engine::RenderLoop(float /* deltaTime */) {
    renderContext->GetRenderer().BeginFrame();
    imgui->BeginFrame();

    renderContext->GetRenderer().Draw(*scene);
    toolbar->Draw();

    imgui->EndFrame();
    renderContext->GetRenderer().EndFrame();
}

Engine::~Engine() = default;

void Engine::LoadPrimitives() {
    auto& registry = scene->GetRegistry();
    auto entity = registry.create();

    std::shared_ptr<Material> material = std::make_shared<Material>();
    material->materialShader = SHV::eShader::kBasicShader;
    Primitive primitive = {material};

    primitive.positions = {{0, 1, 0, 1}, {-1, -1, 0, 1}, {1, -1, 0, 1}};
    primitive.normals = {{0, 1.0}, {0, 1.0}, {0, 1.0}};
    primitive.uvs = {{0, 0}, {0, 0}, {0, 0}};

    primitive.indices = {1, 2, 3};

    auto& renderComponent = registry.emplace<RenderComponent>(entity);
    registry.emplace<TransformComponent>(entity);

    renderComponent.primitive = primitive;
}

void Engine::UnloadPrimitives() {}
/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/LifecycleManager.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of LifecycleManager.
 */

#include "LifecycleManager.h"
#include "../../imgui/imgui.h"
#include "../../imgui/backends/imgui_impl_sdl2.h"
#include "../../imgui/backends/imgui_impl_sdlrenderer2.h"
#include <cstdio>
#include <iostream>

namespace View {

// Minimal Color Palette for Style Setup
namespace Colors {
    const ImVec4 BlackV     = ImVec4(0, 0, 0, 1);
    const ImVec4 SurfaceV   = ImVec4(0.07f, 0.07f, 0.07f, 1);
    const ImVec4 SurfaceLightV = ImVec4(0.16f, 0.16f, 0.16f, 1);
    const ImVec4 HoverV     = ImVec4(0.24f, 0.24f, 0.24f, 1);
    const ImVec4 WhiteV     = ImVec4(1, 1, 1, 1);
    const ImVec4 TextMutedV = ImVec4(0.45f, 0.45f, 0.45f, 1);
    const ImVec4 TransparentV = ImVec4(0, 0, 0, 0);
}

LifecycleManager::LifecycleManager()
    : mWindow(nullptr)
    , mRenderer(nullptr)
    , mInitialized(false)
{}

LifecycleManager::~LifecycleManager() {
    shutdown();
}

bool LifecycleManager::initialize(int width, int height, const char* title) {
    if (mInitialized) return true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!mWindow) return false;

    mRenderer = SDL_CreateRenderer(mWindow, -1, 
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!mRenderer) {
        SDL_DestroyWindow(mWindow);
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplSDL2_InitForSDLRenderer(mWindow, mRenderer);
    ImGui_ImplSDLRenderer2_Init(mRenderer);
    
    // Load font
    io.Fonts->Clear();
    ImFont* font = io.Fonts->AddFontFromFileTTF(
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf", 16.0f);
    if (!font) {
        io.Fonts->AddFontDefault();
    }

    setupStyle();
    mInitialized = true;
    return true;
}

void LifecycleManager::shutdown() {
    if (!mInitialized) return;

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    if (mRenderer) {
        SDL_DestroyRenderer(mRenderer);
        mRenderer = nullptr;
    }
    
    if (mWindow) {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }
    
    // SDL_Quit() is managed by AudioPlayer slightly... 
    // but safe to call SDL_QuitSubSystem(SDL_INIT_VIDEO) here if we wanted
    // For now we just destroy window/renderer
    
    mInitialized = false;
}

void LifecycleManager::processEvents(bool& running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        if (e.type == SDL_QUIT) running = false;
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)
            running = false;
    }
}

void LifecycleManager::beginFrame() {
    if (!mInitialized) return;
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void LifecycleManager::endFrame() {
    if (!mInitialized) return;
    // Rendering is done by View Facade or Components calling ImGui commands
    // Actual render pass is explicit
}

int LifecycleManager::getWindowWidth() const {
    int w, h;
    if (mWindow) {
        SDL_GetWindowSize(mWindow, &w, &h);
        return w;
    }
    return 800;
}

int LifecycleManager::getWindowHeight() const {
    int w, h;
    if (mWindow) {
        SDL_GetWindowSize(mWindow, &w, &h);
        return h;
    }
    return 600;
}

void LifecycleManager::setupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.WindowRounding = 0.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    
    style.WindowPadding = ImVec2(0, 0);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 4);
    style.ScrollbarSize = 8.0f;
    style.GrabMinSize = 8.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    
    ImVec4* c = style.Colors;
    c[ImGuiCol_WindowBg] = Colors::BlackV;
    c[ImGuiCol_ChildBg] = Colors::TransparentV;
    c[ImGuiCol_FrameBg] = Colors::SurfaceLightV;
    c[ImGuiCol_FrameBgHovered] = Colors::HoverV;
    c[ImGuiCol_FrameBgActive] = Colors::HoverV;
    c[ImGuiCol_ScrollbarBg] = Colors::TransparentV;
    c[ImGuiCol_ScrollbarGrab] = Colors::HoverV;
    c[ImGuiCol_SliderGrab] = Colors::WhiteV;
    c[ImGuiCol_SliderGrabActive] = Colors::WhiteV;
    c[ImGuiCol_Button] = Colors::SurfaceLightV;
    c[ImGuiCol_ButtonHovered] = Colors::HoverV;
    c[ImGuiCol_ButtonActive] = Colors::HoverV;
    c[ImGuiCol_Header] = Colors::SurfaceLightV;
    c[ImGuiCol_HeaderHovered] = Colors::HoverV;
    c[ImGuiCol_Text] = Colors::WhiteV;
    c[ImGuiCol_TextDisabled] = Colors::TextMutedV;
}

} // namespace View

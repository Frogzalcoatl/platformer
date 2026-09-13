#include "system/window_manager.h"
#include <box2d/box2d.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

WindowManager::WindowManager(const char* window_name, SDL_Color background_color)
    : background_color(background_color) {
    size_ = WindowVec2{1280, 720};
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED;
#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS)
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    flags |= SDL_WINDOW_FULLSCREEN;
#endif
    is_fullscreen_ = ((flags & SDL_WINDOW_FULLSCREEN) != 0); // Returns true if fullscreen bit is 1
    sdl_window_ = UniqueWindow(SDL_CreateWindow(window_name, size_.x, size_.y, flags));
    if (!sdl_window_) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, "Unable to create SDL3 Window: %s", SDL_GetError()
        );
        return;
    }
    SDL_DisplayID display = SDL_GetDisplayForWindow(sdl_window_.get());
    SDL_Rect display_bounds;
    if (SDL_GetDisplayBounds(display, &display_bounds)) {
        // Just in case the user has a screen smaller than 720p for whatever reason.
        if (size_.x > display_bounds.w) {
            size_.x = display_bounds.w;
        }
        if (size_.y > display_bounds.h) {
            size_.y = display_bounds.h;
        }
    }
    SDL_Log("Created SDL3 Window with name \"%s\"", window_name);
    int actual_width = 0;
    int actual_height = 0;
    SDL_GetWindowSize(sdl_window_.get(), &actual_width, &actual_height);
    size_.x = actual_width;
    size_.y = actual_height;
    SDL_Log("Initialized window size to: %d, %d", size_.x, size_.y);
    sdl_renderer_ = UniqueRenderer(SDL_CreateRenderer(sdl_window_.get(), nullptr));
    if (!sdl_renderer_) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Unable to create SDL3 renderer: %s", SDL_GetError());
        return;
    }
    SDL_Log("Created SDL3 renderer");
    ImGui_ImplSDL3_InitForSDLRenderer(sdl_window_.get(), sdl_renderer_.get());
    ImGui_ImplSDLRenderer3_Init(sdl_renderer_.get());
}

void WindowManager::clear_frame() {
    Uint64 now = SDL_GetTicksNS();
    delta_time_ = static_cast<float>(now - last_frame_time_ns_) / 1000000000.f;
    last_frame_time_ns_ = now;
    if (delta_time_ > 0.1f) {
        delta_time_ = 0.1f;
    }
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    SDL_SetRenderDrawColor(
        sdl_renderer_.get(),
        background_color.r,
        background_color.g,
        background_color.b,
        background_color.a
    );
    SDL_RenderClear(sdl_renderer_.get());
}

void WindowManager::render(Uint64 frame_start_ns) {
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), sdl_renderer_.get());
    SDL_RenderPresent(sdl_renderer_.get());
    if (fps_unlimited_) {
        return;
    }
#ifndef SDL_PLATFORM_ANDROID
    if (vsync_) {
        return;
    }
#endif
    const Uint64 frame_time_ns = SDL_GetTicksNS() - frame_start_ns;
    if (frame_time_ns < target_frame_time_ns_) {
        const Uint64 delay_ns = target_frame_time_ns_ - frame_time_ns;
        SDL_DelayPrecise(delay_ns);
    }
}

SDL_Window* WindowManager::get_sdl_window() const {
    return sdl_window_.get();
}

SDL_Renderer* WindowManager::get_sdl_renderer() const {
    return sdl_renderer_.get();
}

WindowVec2 WindowManager::get_size() const {
    return size_;
}

SDL_Rect WindowManager::get_safe_area() const {
    SDL_Rect safe_area;
    if (!SDL_GetWindowSafeArea(sdl_window_.get(), &safe_area)) {
        return SDL_Rect{0, 0, size_.x, size_.y};
    }
    return safe_area;
}

void WindowManager::handle_resize(int size_x, int size_y) {
    size_.x = size_x;
    size_.y = size_y;
}

WindowVec2 WindowManager::get_mouse_pos() const {
    return mouse_pos_;
}

void WindowManager::handle_mouse_motion_event(const SDL_MouseMotionEvent& event) {
    mouse_pos_.x = static_cast<int>(event.x);
    mouse_pos_.y = static_cast<int>(event.y);
}

Uint64 WindowManager::get_target_fps() const {
    return target_fps_;
}

float WindowManager::get_monitor_refresh_rate() const {
    SDL_DisplayID display_id = SDL_GetDisplayForWindow(sdl_window_.get());
    if (display_id == 0) {
        return 60.f;
    }
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display_id);
    if (!mode) {
        return 60.f;
    }
    return mode->refresh_rate;
}

std::string WindowManager::target_fps_str() const {
    if (vsync_) {
        float vsync_fps = get_monitor_refresh_rate();
        return std::to_string(static_cast<int>(SDL_roundf(vsync_fps))) + ".0";
    } else if (fps_unlimited_) {
        return "Unlimited";
    } else {
        return std::to_string(target_fps_) + ".0";
    }
}

void WindowManager::set_target_fps(Uint64 value) {
    if (value == 0) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Ignoring attempt to set target fps to 0. It is still set to %d",
            static_cast<int>(target_fps_)
        );
        return;
    }
    target_fps_ = value;
    target_frame_time_ns_ = 1000000000ULL / target_fps_;
    SDL_Log("Target fps set to %zu", value);
}

bool WindowManager::is_vsync_enabled() const {
    return vsync_;
}

void WindowManager::set_vsync(bool value) {
    if (value && fps_unlimited_) {
        set_fps_unlimited(false);
    }
    int arg = value ? 1 : 0;
    if (!SDL_SetRenderVSync(sdl_renderer_.get(), arg)) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to toggle VSync: %s", SDL_GetError());
        return;
    }
    SDL_Log("Vsync set to %s", value ? "true" : "false");
    vsync_ = value;
#ifdef SDL_PLATFORM_ANDROID
    if (vsync_) {
        // Vsync cannot be toggled during runtime on android
        set_target_fps(static_cast<Uint64>(SDL_roundf(get_monitor_refresh_rate())));
    }
#endif
}

bool WindowManager::get_fps_unlimited() const {
    return fps_unlimited_;
}

void WindowManager::set_fps_unlimited(bool value) {
    if (vsync_ && value) {
        set_vsync(false);
    }
    fps_unlimited_ = value;
    SDL_Log("FPS Unlimited set to %s", value ? "true" : "false");
}

bool WindowManager::get_is_fullscreen() const {
    return is_fullscreen_;
}

void WindowManager::toggle_fullscreen() {
    is_fullscreen_ = !is_fullscreen_;
    SDL_SetWindowFullscreen(sdl_window_.get(), is_fullscreen_);
}
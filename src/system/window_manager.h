#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <string>

struct WindowVec2 {
    int x;
    int y;
};

struct SdlWindowDeleter {
    void operator()(SDL_Window* w) const {
        if (w) {
            SDL_DestroyWindow(w);
        }
    }
};

struct SdlRendererDeleter {
    void operator()(SDL_Renderer* r) const {
        if (r) {
            SDL_DestroyRenderer(r);
        }
    }
};

using UniqueWindow = std::unique_ptr<SDL_Window, SdlWindowDeleter>;
using UniqueRenderer = std::unique_ptr<SDL_Renderer, SdlRendererDeleter>;

class WindowManager {
  private:
    UniqueWindow sdl_window_;
    UniqueRenderer sdl_renderer_;
    WindowVec2 size_;
    WindowVec2 mouse_pos_;
    Uint64 target_fps_ = 120;
    Uint64 target_frame_time_ns_ = 1000000000ULL / target_fps_;
    Uint64 last_frame_time_ns_ = 0;
    float delta_time_ = 0.f;
    bool vsync_ = true;
    bool fps_unlimited_ = false;
    bool is_fullscreen_ = false;

  public:
    SDL_Color background_color;

    WindowManager(const char* window_name, SDL_Color background_color);

    void clear_frame();

    void render(Uint64 frame_start_ns);

    SDL_Window* get_sdl_window() const;

    SDL_Renderer* get_sdl_renderer() const;

    WindowVec2 get_size() const;

    SDL_Rect get_safe_area() const;

    void handle_resize(int size_x, int size_y);

    WindowVec2 get_mouse_pos() const;

    void handle_mouse_motion_event(const SDL_MouseMotionEvent& event);

    Uint64 get_target_fps() const;

    float get_monitor_refresh_rate() const;

    std::string target_fps_str() const;

    void set_target_fps(Uint64 value);

    float get_delta_time() const {
        return delta_time_;
    }

    bool is_vsync_enabled() const;

    void set_vsync(bool value);

    bool get_fps_unlimited() const;

    void set_fps_unlimited(bool value);

    bool get_is_fullscreen() const;

    void toggle_fullscreen();
};
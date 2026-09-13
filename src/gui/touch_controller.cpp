#include "gui/touch_controller.h"
#include "assets/asset_paths.h"
#include "gui/imgui_widgets.h"
#include "platformer/game_events.h"
#include <vector>

TouchController::TouchController() {
    entity_controller_.is_sprinting = true;
}

TouchController::TouchController(Entity& entity) : entity_controller_(entity) {
    entity_controller_.is_sprinting = true;
}

bool TouchController::is_last_item_touched(const std::vector<ImVec2>& touch_positions) {
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    for (const ImVec2& pos : touch_positions) {
        if (pos.x >= min.x && pos.x <= max.x && pos.y >= min.y && pos.y <= max.y) {
            return true;
        }
    }
    if (touch_positions.empty()) {
        return ImGui::IsItemActive();
    }
    return false;
}

void TouchController::draw(WindowManager& window, float ui_scale) {
    ImGui::Begin(
        "Touch Controls",
        nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize
    );
    WindowVec2 window_size = window.get_size();
    ImVec2 window_size_f{static_cast<float>(window_size.x), static_cast<float>(window_size.y)};
    std::vector<ImVec2> active_touch_positions;
    int touch_device_count = 0;
    SDL_TouchID* touch_devices = SDL_GetTouchDevices(&touch_device_count);
    if (touch_devices) {
        for (int i = 0; i < touch_device_count; i++) {
            SDL_TouchDeviceType type = SDL_GetTouchDeviceType(touch_devices[i]);
            if (type != SDL_TOUCH_DEVICE_DIRECT) {
                continue;
            }
            int finger_count;
            SDL_Finger** fingers = SDL_GetTouchFingers(touch_devices[i], &finger_count);
            if (fingers) {
                for (int j = 0; j < finger_count; j++) {
                    // Multiply by window size since the coordinates are from 0.0-1.0
                    active_touch_positions.push_back(
                        ImVec2{fingers[j]->x * window_size_f.x, fingers[j]->y * window_size_f.y}
                    );
                }
                SDL_free(fingers);
            }
        }
        SDL_free(touch_devices);
    }
    SDL_Rect safe_area = window.get_safe_area();
    SDL_FRect safe_area_f{
        static_cast<float>(safe_area.x),
        static_cast<float>(safe_area.y),
        static_cast<float>(safe_area.w),
        static_cast<float>(safe_area.h)
    };
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 0.40f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 0.60f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.40f, 0.40f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 1.00f, 1.00f, 0.45f));
    std::vector<TouchButtonRect> button_rects;

    ImVec2 pause_button_size{100.f * ui_scale, 100.f * ui_scale};
    ImVec2 pause_button_pos{window_size_f.x / 2.f - pause_button_size.x, 0.f};
    ImGui::SetCursorPos(pause_button_pos);
    imgui_widgets::custom_pause_button("##Pause", pause_button_size);
    button_rects.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax()});

    bool is_pause_touched = is_last_item_touched(active_touch_positions);
    if (is_pause_touched && !was_pause_touched_) {
        game_events::push(
            game_event_types::Input{
                InputVerb::pause, InputState::pressed, InputSource{InputType::touch, 0}
            }
        );
    }
    was_pause_touched_ = is_pause_touched;

    ImVec2 button_size = ImVec2{100.f * ui_scale, 100.f * ui_scale};
    ImVec2 up_button_pos{
        safe_area_f.w - button_size.x, safe_area_f.y + safe_area_f.h - button_size.y * 2.f
    };
    ImGui::SetCursorPos(up_button_pos);
    imgui_widgets::custom_arrow_button("##Up", ImGuiDir_Up, button_size);
    button_rects.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax()});

    bool is_up_touched = is_last_item_touched(active_touch_positions);
    if (is_up_touched && !was_up_touched_) {
        entity_controller_.movement[static_cast<size_t>(EntityMovement::up)] = true;
        entity_controller_.jump();
        float pitch = SDL_randf() * (1.25f - 1.f) + 1.f;
        game_events::push(game_event_types::PlaySound{asset_paths::sounds::jump, 100, pitch});
    } else if (!is_up_touched && was_up_touched_) {
        entity_controller_.movement[static_cast<size_t>(EntityMovement::up)] = false;
    }
    was_up_touched_ = is_up_touched;

    ImVec2 down_button_pos{up_button_pos.x, up_button_pos.y + button_size.y};
    ImGui::SetCursorPos(down_button_pos);
    imgui_widgets::custom_arrow_button("##Down", ImGuiDir_Down, button_size);
    button_rects.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax()});
    entity_controller_.movement[static_cast<size_t>(EntityMovement::down)] =
        is_last_item_touched(active_touch_positions);

    ImVec2 left_button_pos{static_cast<float>(safe_area.x), down_button_pos.y};
    ImGui::SetCursorPos(left_button_pos);
    imgui_widgets::custom_arrow_button("##Left", ImGuiDir_Left, button_size);
    button_rects.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax()});
    entity_controller_.movement[static_cast<size_t>(EntityMovement::left)] =
        is_last_item_touched(active_touch_positions);

    ImVec2 right_button_pos{left_button_pos.x + button_size.x, left_button_pos.y};
    ImGui::SetCursorPos(right_button_pos);
    imgui_widgets::custom_arrow_button("##Right", ImGuiDir_Right, button_size);
    button_rects.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax()});
    entity_controller_.movement[static_cast<size_t>(EntityMovement::right)] =
        is_last_item_touched(active_touch_positions);

    ImGui::PopStyleColor(4);
    ImGui::End();

    int ui_fingers = 0;
    for (const ImVec2& pos : active_touch_positions) {
        bool on_ui = false;
        for (const auto& rect : button_rects) {
            if (pos.x >= rect.min.x && pos.x <= rect.max.x && pos.y >= rect.min.y &&
                pos.y <= rect.max.y) {
                on_ui = true;
                break;
            }
        }
        if (on_ui) {
            ui_fingers++;
        }
    }
    free_finger_count_ = static_cast<int>(active_touch_positions.size()) - ui_fingers;
    entity_controller_.update();
}
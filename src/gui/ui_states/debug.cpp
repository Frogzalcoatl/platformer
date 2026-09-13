#include "gui/ui_manager.h"

void UiManager::draw_debug(
    WindowManager& window, Entity* player_entity, Camera* camera, InputManager& input, Level* level
) {
    ImGui::PushFont(font_small_);
    SDL_Rect safe_area = window.get_safe_area();
    ImGui::SetNextWindowPos(
        ImVec2{static_cast<float>(safe_area.x), static_cast<float>(safe_area.y)}, ImGuiCond_Always
    );
    if (ImGui::Begin(
            "Debug Menu",
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_AlwaysAutoResize
        )) {
        ImGui::Text("Window:");
        WindowVec2 window_size = window.get_size();
        ImGui::Text("Size: %d, %d", window_size.x, window_size.y);
        ImGui::Text("Framerate:");
        ImGui::SameLine();
        fps_text(window);
        ImGui::Dummy(ImVec2{1.f, 1.f});
        ImGui::Text("\nUI State: %s", get_state_str().c_str());
        ImGui::Dummy(ImVec2{1.f, 1.f});
        if (level) {
            std::string_view level_name = level->get_name();
            LevelDimensions level_size = level->get_size();
            LevelDrawInfo draw_info = level->drawn_last_frame();
            size_t tile_count = level->get_tile_count();
            size_t entities_count = level->get_entities().size();
            ImGui::Text("\nLevel:");
            ImGui::Text(
                // %.*s tells the func to read exactly N characters, preventing it from running past
                // the end of a string_view
                "Name: \"%.*s\"\nSize: (%zu, %zu)\nTiles Drawn: %zu/%zu\nEntities Drawn: %zu/%zu",
                static_cast<int>(level_name.length()),
                level_name.data(),
                level_size.width,
                level_size.height,
                draw_info.tiles,
                tile_count,
                draw_info.entities,
                entities_count
            );
            ImGui::Dummy(ImVec2{1.f, 1.f});
        }
        if (player_entity) {
            b2Vec2 position = b2Body_GetPosition(player_entity->get_body_id());
            b2Vec2 velocity = b2Body_GetLinearVelocity(player_entity->get_body_id());
            ImGui::Text("\nPlayer 1:");
            ImGui::Text(
                "Position: %.2f, %.2f\nVelocity: %.2f, %.2f",
                position.x,
                position.y,
                velocity.x,
                velocity.y
            );
            ImGui::Dummy(ImVec2{1.f, 1.f});
        }
        if (camera) {
            const b2Vec2 offset_world = camera->get_offset_world();
            const WindowVec2 offset_pixels = camera->get_offset_pixels();
            const b2Vec2 size = camera->get_size();
            const b2Vec2 safe_area_size = camera->get_safe_area_size();
            const b2Vec2 safe_area_value = camera->get_entity_safe_area_value();
            const b2Vec2 mouse_world_pos = camera->pixel_pos_to_world_pos(window.get_mouse_pos());
            const float scale_multiplier = camera->get_scale_multiplier();
            ImGui::Text("\nCamera:");
            ImGui::Text(
                "Zoom: %.2f\nOffset Pixels: %d, %d\nOffset World: %.2f, %.2f\nSize World: %.2f, %.2f\nSafe Area Size World: %.2f, %.2f\nPlayer Ratio from Center: %.2f, %.2f\nMouse Position World: %.2f, %.2f",
                scale_multiplier,
                offset_pixels.x,
                offset_pixels.y,
                offset_world.x,
                offset_world.y,
                size.x,
                size.y,
                safe_area_size.x,
                safe_area_size.y,
                safe_area_value.x,
                safe_area_value.y,
                mouse_world_pos.x,
                mouse_world_pos.y
            );
            ImGui::Dummy(ImVec2{1.f, 1.f});
        }
        int sdl_gamepad_count = input.sdl_gamepads_detected();
        ImGui::Text("\nSDL Gamepads Detected: %d", sdl_gamepad_count);
        size_t player_source_count = input.get_player_source_count();
        size_t max_player_source_count = input.get_player_sources().size();
        ImGui::Text(
            "Player Sources Connected: %zu/%zu", player_source_count, max_player_source_count
        );
        ImGui::PopFont();
    }
    ImGui::End();
}
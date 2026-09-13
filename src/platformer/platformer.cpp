#include "platformer/platformer.h"
#include "system/discord_rpc_manager.h"
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <vector>

#ifdef SDL_PLATFORM_ANDROID
#include "system/android.h"
#endif

Platformer::Platformer()
    : window_{"C++ Platformer", colors::background}, assets_(window_.get_sdl_renderer()),
      audio_(assets_), settings_("Settings.json"), ui_(assets_) {
    load_settings();
    assets_.add_controller_mappings("gamepads/gamecontrollerdb.txt");
    assets_.add_controller_mappings("gamepads/retrolink.txt");
    MIX_Mixer* mixer_device = audio_.get_mixer_device();
    if (mixer_device) {
        assets_.load_audio(asset_paths::sounds::hover, mixer_device, true);
        assets_.load_audio(asset_paths::sounds::click, mixer_device, true);
    }
}

void Platformer::load_settings() {
    const Settings& current_settings = settings_.get();
#ifdef SDL_PLATFORM_ANDROID
    window_.set_vsync(false);
#else
    window_.set_vsync(current_settings.vsync_enabled);
#endif
    const unsigned int min_target_fps = 1;
    const unsigned int max_target_fps = 1000;
    if (current_settings.target_fps < min_target_fps) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Clamping target fps from %u to %u",
            current_settings.target_fps,
            min_target_fps
        );
        settings_.set_target_fps(min_target_fps);
    } else if (current_settings.target_fps > max_target_fps) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Clamping target fps from %u to %u",
            current_settings.target_fps,
            max_target_fps
        );
        settings_.set_target_fps(max_target_fps);
    }
    if (settings_.created_new_file_on_read()) {
        const Uint64 monitor_refresh_rate =
            static_cast<Uint64>(SDL_roundf(window_.get_monitor_refresh_rate()));
        settings_.set_target_fps(static_cast<unsigned int>(monitor_refresh_rate));
        window_.set_target_fps(monitor_refresh_rate);
    } else {
        window_.set_target_fps(current_settings.target_fps);
    }
    window_.set_fps_unlimited(current_settings.fps_unlimited);
    ui_.set_scale_index(current_settings.ui_scale);
    size_t user_preferred_scale = ui_.get_scale_index();
    if (user_preferred_scale != current_settings.ui_scale) {
        settings_.set_ui_scale(user_preferred_scale);
    }
    const unsigned int max_volume = 200;
    if (current_settings.master_volume > max_volume) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Clamping master volume from %u to %u",
            current_settings.master_volume,
            max_volume
        );
        settings_.set_master_volume(max_volume);
    }
    if (current_settings.sounds_volume > max_volume) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Clamping sounds volume from %u to %u",
            current_settings.sounds_volume,
            max_volume
        );
        settings_.set_sounds_volume(max_volume);
    }
    if (current_settings.music_volume > max_volume) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Clamping music volume from %u to %u",
            current_settings.music_volume,
            max_volume
        );
        settings_.set_music_volume(max_volume);
    }
    audio_.set_volume(AudioCategory::master, current_settings.master_volume);
    audio_.set_volume(AudioCategory::sounds, current_settings.sounds_volume);
    audio_.set_volume(AudioCategory::music, current_settings.music_volume);
}

void Platformer::handle_sdl_event() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        switch (event.type) {
        case SDL_EVENT_QUIT: {
            running_ = false;
#ifdef SDL_PLATFORM_ANDROID
            android::quit_and_remove_task();
#endif
        } break;
        case SDL_EVENT_WINDOW_RESIZED: {
            window_.handle_resize(event.window.data1, event.window.data2);
            if (current_level_) {
                Camera* camera = current_level_->get_camera();
                if (camera) {
                    camera->handle_window_resize(event.window.data1, event.window.data2);
                }
            }
        } break;
        case SDL_EVENT_MOUSE_MOTION: {
            window_.handle_mouse_motion_event(event.motion);
        } break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_MOUSE_WHEEL:
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            std::vector<game_event_types::Input> input_events =
                input_.get_input_events_from_sdl_event(event);
            for (const auto& input_event : input_events) {
                game_events::push(input_event);
            }
        } break;
        case SDL_EVENT_PINCH_BEGIN:
        case SDL_EVENT_PINCH_UPDATE:
        case SDL_EVENT_PINCH_END: {
            if (ui_.get_free_finger_count() >= 2) {
                input_.handle_pinch_event(event.pinch);
            }
        } break;
        case SDL_EVENT_GAMEPAD_ADDED: {
            game_events::schedule(
                game_event_types::GamepadConnectedNotification{event.gdevice.which}, 25
            );
        } break;
        case SDL_EVENT_GAMEPAD_REMOVED: {
            input_.handle_gamepad_removed(event.gdevice);
            std::string message =
                "Controller Disconnected: " + input_.get_gamepad_name(event.gdevice.which);
            notification_manager_.send(message);
        } break;
        case SDL_EVENT_DID_ENTER_BACKGROUND: {
            settings_.save_to_disk();
        } break;
        default:
            break;
        }
    }
}

void Platformer::handle_input_game_event(const game_event_types::Input& input_event) {
    UiState ui_state = ui_.get_state();
    if (input_event.state == InputState::pressed) {
        switch (input_event.verb) {
        case InputVerb::toggle_fullscreen:
#if defined(SDL_PLATFORM_WINDOWS) || defined(SDL_PLATFORM_MACOS) || defined(SDL_PLATFORM_LINUX)
            window_.toggle_fullscreen();
#endif
            break;
        case InputVerb::zoom_in:
            if (current_level_ && ui_state == UiState::playing) {
                Camera* camera = current_level_->get_camera();
                if (camera) {
                    camera->increment_scale_multiplier_smooth(0.05f);
                }
            }
            break;
        case InputVerb::zoom_out:
            if (current_level_ && ui_state == UiState::playing) {
                Camera* camera = current_level_->get_camera();
                if (camera) {
                    camera->increment_scale_multiplier_smooth(-0.05f);
                }
            }
            break;
        case InputVerb::zoom_reset:
            if (current_level_ && ui_state == UiState::playing) {
                Camera* camera = current_level_->get_camera();
                if (camera) {
                    camera->reset_scale_multiplier();
                }
            }
            break;
        case InputVerb::toggle_debug:
            ui_.toggle_debug();
            break;
        case InputVerb::cancel:
            if (ui_state == UiState::playing) {
                break;
            }
            SDL_FALLTHROUGH;
        case InputVerb::pause:
            if (ImGui::IsAnyItemActive()) {
                break;
            }
            ui_.run_cancel_event();
            if (current_level_) {
                const auto& players = current_level_->get_players();
                for (const auto& player : players) {
                    if (player.controller) {
                        player.controller->reset_input();
                    }
                }
            }
            break;
        case InputVerb::show_hitboxes:
            if (current_level_) {
                current_level_->show_hitboxes = !current_level_->show_hitboxes;
            }
            break;
        default:
            break;
        }
    }
    if (current_level_ && ui_state == UiState::playing) {
        current_level_->handle_input(input_event);
    }
    ui_.pass_input_to_imgui(input_event);
}

void Platformer::handle_game_event() {
    game_events::update_scheduled_events();
    GameEvent event;
    while (game_events::poll(event)) {
        if (std::holds_alternative<game_event_types::CloseWindow>(event)) {
            running_ = false;
#ifdef SDL_PLATFORM_ANDROID
            android::quit_and_remove_task();
#endif
        } else if (
            const auto* play_sound_event = std::get_if<game_event_types::PlaySound>(&event)
        ) {
            audio_.play_sound(
                play_sound_event->relative_path, play_sound_event->volume, play_sound_event->pitch
            );
        } else if (
            const auto* play_music_event = std::get_if<game_event_types::PlayMusic>(&event)
        ) {
            audio_.play_music(
                play_music_event->relative_path,
                play_music_event->volume,
                play_music_event->pitch,
                play_music_event->loop
            );
        } else if (const auto* set_volume = std::get_if<game_event_types::SetVolume>(&event)) {
            audio_.set_volume(set_volume->category, set_volume->volume);
        } else if (const auto* input_event = std::get_if<game_event_types::Input>(&event)) {
            handle_input_game_event(*input_event);
        } else if (const auto* set_ui_state = std::get_if<game_event_types::SetUiState>(&event)) {
            ui_.set_state(set_ui_state->state);
        } else if (
            const auto* set_level_name = std::get_if<game_event_types::SetLevelName>(&event)
        ) {
            const LevelAssetsVector& previous_level_assets =
                current_level_ ? current_level_->get_required_assets() : LevelAssetsVector{};
            if (set_level_name->level == LevelName::none) {
                current_level_->unload_required_assets(assets_);
                current_level_ = nullptr;
                window_.background_color = colors::background;
                continue;
            } else if (set_level_name->level == LevelName::test) {
                current_level_ = get_test_level(assets_, window_, audio_, previous_level_assets);
            }
            current_level_->update_players(input_.get_player_sources(), assets_);
            size_t touch_player_index;
            if (!input_.is_touch_player_enabled(&touch_player_index)) {
                ui_.disable_touch_controller();
            } else {
                Entity* touch_entity = current_level_->get_player_entity(touch_player_index);
                if (touch_entity) {
                    ui_.enable_touch_controller(*touch_entity);
                } else {
                    ui_.disable_touch_controller();
                }
            }
            window_.background_color = current_level_->background_color;
        } else if (
            const auto* player_source_added =
                std::get_if<game_event_types::PlayerSourceAdded>(&event)
        ) {
            if (current_level_) {
                current_level_->update_players(input_.get_player_sources(), assets_);
            }
            ui_.set_player_source_added_this_frame(true);
            if (ui_.get_state() != UiState::player_source_setup) {
                std::string notification = "Player Source Added: \"" +
                                           input_.get_source_name(player_source_added->source) +
                                           "\"";
                notification_manager_.send(notification);
            }
        } else if (
            const auto* player_source_removed =
                std::get_if<game_event_types::PlayerSourceRemoved>(&event)
        ) {
            if (current_level_) {
                current_level_->update_players(input_.get_player_sources(), assets_);
            }
            if (ui_.get_state() != UiState::player_source_setup) {
                std::string notification = "Player Source Removed: \"" +
                                           input_.get_source_name(player_source_removed->source) +
                                           "\"";
                notification_manager_.send(notification);
            }
        } else if (
            const auto* detect_new_players =
                std::get_if<game_event_types::ShouldDetectNewPlayerSources>(&event)
        ) {
            if (detect_new_players->value) {
                input_.listen_for_valid_keyboard = true;
                input_.listen_for_new_gamepad = true;
                input_.enable_touch_player();
                SDL_Log("Enabled input detection for adding new player sources.");
            } else {
                bool value_will_be_changed =
                    input_.listen_for_valid_keyboard || input_.listen_for_new_gamepad;
                input_.listen_for_valid_keyboard = false;
                input_.listen_for_new_gamepad = false;
                if (value_will_be_changed) {
                    SDL_Log("Disabled input detection for adding new player sources.");
                }
            }
        } else if (
            const auto* change_level_zoom = std::get_if<game_event_types::ChangeLevelZoom>(&event)
        ) {
            if (current_level_ && ui_.get_state() == UiState::playing) {
                Camera* camera = current_level_->get_camera();
                if (camera) {
                    if (change_level_zoom->smooth) {
                        camera->increment_scale_multiplier_smooth(change_level_zoom->amount);
                    } else {
                        camera->increment_scale_multiplier(change_level_zoom->amount);
                    }
                }
            }
        } else if (
            const auto* send_notification = std::get_if<game_event_types::SendNotification>(&event)
        ) {
            notification_manager_.send(send_notification->message, send_notification->on_click);
        } else if (
            const auto* gamepad_event_notification =
                std::get_if<game_event_types::GamepadConnectedNotification>(&event)
        ) {
            std::string message =
                "Controller Connected: " + input_.get_gamepad_name(gamepad_event_notification->id);
            notification_manager_.send(message);
        } else if (
            const auto* save_user_data = std::get_if<game_event_types::SaveUserData>(&event)
        ) {
            if (save_user_data->type == UserDataTypes::settings) {
                settings_.save_to_disk();
            }
        }
    }
}

// Just for testing
static bool play_music_failed = false;
const std::vector<const char*> music_file_names = {
    "2023_4 (unfinished).ogg",
    "2023_11(3).ogg",
    "2023_14.ogg",
    "2023_23.ogg",
    "2023_29.ogg",
    "2023_35.ogg",
    "2023_37.ogg",
    "2024_3.ogg",
    "2024_5.ogg",
    "2024_7.ogg",
    "2024_8.ogg",
    "2026_2.ogg",
    "2026_3.ogg",
    "2026_4.ogg",
    "2026_5.ogg",
    "2026_6.ogg"
};

void Platformer::run() {
    running_ = true;
    while (running_) {
        if (!audio_.is_music_playing() && !play_music_failed) {
            size_t random_song_id = static_cast<size_t>(
                SDL_floorf(static_cast<float>(music_file_names.size()) * SDL_randf())
            );
            std::string relative_path = "music/";
            relative_path += music_file_names[random_song_id];
            if (!audio_.play_music(relative_path, 100, audio_.get_music_pitch(), false)) {
                play_music_failed = true;
            }
        }
        const Uint64 frame_start_ns = SDL_GetTicksNS();
        handle_sdl_event();
        handle_game_event();
        window_.clear_frame();
        if (current_level_) {
            UiState current_state = ui_.get_state();
            if (current_state == UiState::playing) {
                current_level_->update();
            }
            current_level_->draw(window_, assets_);
        }
        ui_.update(window_, settings_, audio_, input_, current_level_.get());
        notification_manager_.update(window_, ui_.get_actual_scale());
        window_.render(frame_start_ns);
        discord_rpc_manager::update();
    }
    settings_.save_to_disk();
}
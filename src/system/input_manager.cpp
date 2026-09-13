#include "system/input_manager.h"
#include <algorithm>
#include <cassert>
#include <imgui.h>

std::string input_verb_to_string(InputVerb verb) {
    switch (verb) {
    case InputVerb::up:
        return "Up";
    case InputVerb::down:
        return "Down";
    case InputVerb::left:
        return "Left";
    case InputVerb::right:
        return "Right";
    case InputVerb::jump:
        return "Jump";
    case InputVerb::sprint:
        return "Sprint";
    case InputVerb::respawn:
        return "Respawn";
    case InputVerb::confirm:
        return "Confirm";
    case InputVerb::cancel:
        return "Cancel";
    case InputVerb::pause:
        return "Pause Game";
    case InputVerb::zoom_in:
        return "Zoom In";
    case InputVerb::zoom_out:
        return "Zoom Out";
    case InputVerb::zoom_reset:
        return "Zoom Reset";
    case InputVerb::toggle_fullscreen:
        return "Toggle Fullscreen";
    case InputVerb::toggle_debug:
        return "Toggle Debug";
    case InputVerb::show_hitboxes:
        return "Show Hitboxes";
    default:
        return "";
    }
}

std::string input_type_to_string(InputType type) {
    switch (type) {
    case InputType::controller:
        return "Controller";
    case InputType::keyboard:
        return "Keyboard";
    case InputType::mouse:
        return "Mouse";
    case InputType::touch:
        return "Touch";
    default:
        return "Unknown Input Type";
    }
}

InputManager::InputManager() {
    for (size_t i = 0; i < static_cast<size_t>(InputVerb::verb_count); i++) {
        gamepad_bindings_[i].fill(SDL_GAMEPAD_BUTTON_INVALID);
    }
    for (const auto& binding : default_verb_bindings) {
        bind_scancode_to_verb(
            binding.verb, ScancodeInfo{binding.scancode, binding.activate_on_repeat}, std::nullopt
        );
    }
    for (const auto& binding : default_gamepad_bindings) {
        bind_gamepad_button_to_verb(binding.verb, binding.button, std::nullopt);
    }
}

void InputManager::bind_scancode_to_verb(
    InputVerb verb, ScancodeInfo scancode_info, std::optional<size_t> at_index_opt
) {
    assert(verb < InputVerb::verb_count);
    if (scancode_info.scancode <= SDL_SCANCODE_UNKNOWN ||
        scancode_info.scancode >= SDL_SCANCODE_COUNT) {
        return;
    }
    // Using a reference instead of a pointer.
    /*
    Differences between reference and pointer:
    - References are not a memory address, rather a direct reference to an existing variable.
    - References are never null since they are attached to existing values.
    - reassigning the below verbBinds variable to something else would also overwrite the array
    above.
    - Cleaner syntax
    */
    auto& bindings = scancode_bindings_[static_cast<size_t>(verb)];
    if (at_index_opt.has_value()) {
        const size_t& at_index = at_index_opt.value();
        assert(at_index < max_binds_per_verb);
        bindings[at_index] = scancode_info;
        return;
    }
    for (size_t i = 0; i < max_binds_per_verb; i++) {
        if (bindings[i].scancode == scancode_info.scancode) {
            bindings[i].activate_on_repeat = scancode_info.activate_on_repeat;
            return;
        }
    }
    for (size_t i = 0; i < max_binds_per_verb; i++) {
        if (bindings[i].scancode == SDL_SCANCODE_UNKNOWN) {
            bindings[i] = scancode_info;
            return;
        }
    }
}

void InputManager::unbind_scancode_from_verb(InputVerb verb, SDL_Scancode scancode) {
    assert(verb < InputVerb::verb_count);
    if (scancode <= SDL_SCANCODE_UNKNOWN || scancode >= SDL_SCANCODE_COUNT) {
        return;
    }
    auto& bindings = scancode_bindings_[static_cast<size_t>(verb)];
    for (size_t i = 0; i < max_binds_per_verb; i++) {
        if (bindings[i].scancode == scancode) {
            bindings[i].scancode = SDL_SCANCODE_UNKNOWN;
            bindings[i].activate_on_repeat = false;
        }
    }
}

void InputManager::clear_scancode_binding_at_index(InputVerb verb, size_t index) {
    assert(verb < InputVerb::verb_count);
    assert(index < max_binds_per_verb);
    auto& bindings = scancode_bindings_[static_cast<size_t>(verb)];
    bindings[index].scancode = SDL_SCANCODE_UNKNOWN;
    bindings[index].activate_on_repeat = false;
}

std::vector<InputVerbInfo> InputManager::get_verbs_from_scancode(SDL_Scancode scancode) {
    std::vector<InputVerbInfo> input_verbs = {};
    if (scancode <= SDL_SCANCODE_UNKNOWN || scancode >= SDL_SCANCODE_COUNT) {
        return input_verbs;
    }
    for (size_t i = 0; i < static_cast<size_t>(InputVerb::verb_count); i++) {
        for (ScancodeInfo& binding : scancode_bindings_[i]) {
            if (binding.scancode == scancode) {
                input_verbs.push_back(
                    InputVerbInfo{static_cast<InputVerb>(i), binding.activate_on_repeat}
                );
            }
        }
    }
    return input_verbs;
}

const std::array<ScancodeInfo, max_binds_per_verb>&
InputManager::get_scancodes_from_verb(InputVerb verb) const {
    assert(verb < InputVerb::verb_count);
    return scancode_bindings_[static_cast<size_t>(verb)];
}

const ScancodeBindings& InputManager::get_scancode_bindings() const {
    return scancode_bindings_;
}

void InputManager::bind_gamepad_button_to_verb(
    InputVerb verb, SDL_GamepadButton button, std::optional<size_t> at_index_opt
) {
    assert(verb < InputVerb::verb_count);
    if (button <= SDL_GAMEPAD_BUTTON_INVALID || button >= SDL_GAMEPAD_BUTTON_COUNT) {
        return;
    }
    auto& bindings = gamepad_bindings_[static_cast<size_t>(verb)];
    if (at_index_opt.has_value()) {
        const size_t& at_index = at_index_opt.value();
        assert(at_index < max_binds_per_verb);
        bindings[at_index] = button;
        return;
    }
    for (size_t i = 0; i < max_binds_per_verb; i++) {
        if (bindings[i] == button) {
            return;
        }
    }
    for (size_t i = 0; i < max_binds_per_verb; i++) {
        if (bindings[i] == SDL_GAMEPAD_BUTTON_INVALID) {
            bindings[i] = button;
            return;
        }
    }
}

void InputManager::unbind_gamepad_button_from_verb(InputVerb verb, SDL_GamepadButton button) {
    assert(verb < InputVerb::verb_count);
    if (button <= SDL_GAMEPAD_BUTTON_INVALID || button >= SDL_GAMEPAD_BUTTON_COUNT) {
        return;
    }
    auto& bindings = gamepad_bindings_[static_cast<size_t>(verb)];
    for (size_t i = 0; i < max_binds_per_verb; i++) {
        if (bindings[i] == button) {
            bindings[i] = SDL_GAMEPAD_BUTTON_INVALID;
        }
    }
}

void InputManager::clear_gamepad_button_binding_at_index(InputVerb verb, size_t index) {
    assert(verb < InputVerb::verb_count);
    assert(index < max_binds_per_verb);
    auto& bindings = gamepad_bindings_[static_cast<size_t>(verb)];
    bindings[index] = SDL_GAMEPAD_BUTTON_INVALID;
}

std::vector<InputVerb> InputManager::get_verbs_from_gamepad_button(SDL_GamepadButton button) {
    std::vector<InputVerb> verbs = {};
    if (button <= SDL_GAMEPAD_BUTTON_INVALID || button >= SDL_GAMEPAD_BUTTON_COUNT) {
        return verbs;
    }
    for (size_t i = 0; i < static_cast<size_t>(InputVerb::verb_count); i++) {
        for (SDL_GamepadButton& button_binding : gamepad_bindings_[i]) {
            if (button_binding == button) {
                verbs.push_back(static_cast<InputVerb>(i));
            }
        }
    }
    return verbs;
}

const std::array<SDL_GamepadButton, max_binds_per_verb>&
InputManager::get_gamepad_buttons_from_verb(InputVerb verb) const {
    assert(verb < InputVerb::verb_count);
    return gamepad_bindings_[static_cast<size_t>(verb)];
}

const GamepadBindings& InputManager::get_gamepad_bindings() const {
    return gamepad_bindings_;
}

const PlayerSources& InputManager::get_player_sources() const {
    return player_sources_;
}

size_t InputManager::get_player_source_count() const {
    return player_source_count_;
}

int InputManager::sdl_gamepads_detected() const {
    int count = 0;
    SDL_GetGamepads(&count);
    return count;
}

std::string InputManager::get_source_name(const InputSource& source) {
    if (source.type == InputType::controller) {
        return get_gamepad_name(source.sdl_id);
    }
    return input_type_to_string(source.type);
}

std::string InputManager::get_gamepad_name(const SDL_JoystickID id) {
    SDL_Gamepad* gamepad = SDL_GetGamepadFromID(id);
    const char* name = SDL_GetGamepadName(gamepad);
    if (!name) {
        name = SDL_GetJoystickNameForID(id);
    }
    if (name) {
        return std::string{name};
    }
    return "Controller " + std::to_string(id);
}

bool InputManager::add_player_source(const InputSource& source) {
    assert(source.type < InputType::input_type_count);
    if (player_source_count_ == max_player_sources) {
        return false;
    }
    bool already_added = std::any_of(
        player_sources_.begin(),
        player_sources_.end(),
        [&source](const std::optional<InputSource>& player_source) {
            return source == player_source;
        }
    );
    if (already_added) {
        return false;
    }
    auto empty_it = std::find_if(
        player_sources_.begin(),
        player_sources_.end(),
        [](const std::optional<InputSource>& player_source) { return !player_source.has_value(); }
    );
    std::string source_name = get_source_name(source);
    if (empty_it == player_sources_.end()) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Ignoring attempt to add input source \"%s\": No null index found on playerSources array.",
            source_name.c_str()
        );
        return false;
    }
    *empty_it = source;
    player_source_count_++;
    size_t index = static_cast<size_t>(std::distance(player_sources_.begin(), empty_it));
    game_events::push(game_event_types::PlayerSourceAdded{source, index});
    SDL_Log("Added player source \"%s\" at index %zu", source_name.c_str(), index);
    return true;
}

bool InputManager::remove_player_source(const InputSource& source) {
    auto source_it = std::find(player_sources_.begin(), player_sources_.end(), source);
    std::string source_name = get_source_name(source);
    if (source_it == player_sources_.end()) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Ignoring request to remove input source \"%s\": They are not currently on playerSources array.",
            source_name.c_str()
        );
        return false;
    }
    *source_it = std::nullopt;
    if (player_source_count_ == 0) {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Ignored attempt decrement an unsigned playerSourceCount of 0. May be out of sync."
        );
    } else {
        player_source_count_--;
    }
    // Learned about std::stable_partition from AI
    // stable partition shifts all values that return true to the starting indices
    // shifts all values that return false to the indices after.
    std::stable_partition(
        player_sources_.begin(), player_sources_.end(), [](const std::optional<InputSource>& src) {
            return src.has_value();
        }
    );
    size_t index = static_cast<size_t>(std::distance(player_sources_.begin(), source_it));
    game_events::push(game_event_types::PlayerSourceRemoved{source, index});
    SDL_Log("Removed player source \"%s\" at index %zu", source_name.c_str(), index);
    return true;
}

void InputManager::handle_gamepad_removed(SDL_GamepadDeviceEvent& event) {
    if (event.type != SDL_EVENT_GAMEPAD_REMOVED) {
        return;
    }
    InputSource gamepad_source{InputType::controller, event.which};
    remove_player_source(gamepad_source);
}

bool InputManager::has_touch_screen() {
    int touch_device_count;
    SDL_TouchID* touch_devices = SDL_GetTouchDevices(&touch_device_count);
    if (!touch_devices || touch_device_count == 0) {
        return false;
    }
    bool has_direct_touch = false;
    for (int i = 0; i < touch_device_count; i++) {
        SDL_TouchDeviceType device_type = SDL_GetTouchDeviceType(touch_devices[i]);
        if (device_type == SDL_TOUCH_DEVICE_DIRECT) {
            has_direct_touch = true;
            break;
        }
    }
    SDL_free(touch_devices);
    return has_direct_touch;
}

bool InputManager::enable_touch_player() {
    if (!has_touch_screen()) {
        SDL_Log("Not enabling touch player. No touch screen detected");
        return false;
    }
    return add_player_source(default_touch_source_);
}

bool InputManager::disable_touch_player() {
    return remove_player_source(default_touch_source_);
}

void InputManager::remove_player_source_at_index(size_t index) {
    if (index >= player_sources_.size()) {
        SDL_Log("Unable to remove player at invalid index %zu.", index);
        return;
    }
    if (!player_sources_[index].has_value()) {
        return;
    }
    // This is technically inefficient, but since theres only 4 players it does not matter.
    // Just so i dont have to worry about keeping the logic for removing players in check at two
    // different places.
    remove_player_source(player_sources_[index].value());
}

bool InputManager::is_touch_player_enabled(size_t* at_index) {
    for (size_t i = 0; i < player_sources_.size(); i++) {
        if (!player_sources_[i].has_value()) {
            continue;
        }
        InputSource source = player_sources_[i].value();
        if (source.type == InputType::touch) {
            if (at_index) {
                *at_index = i;
            }
            return true;
        }
    }
    return false;
}

std::vector<game_event_types::Input> InputManager::handle_keyboard_event(SDL_KeyboardEvent& event) {
    if (event.scancode == SDL_SCANCODE_AC_BACK) {
        std::vector<game_event_types::Input> android_input_events;
        InputState input_state =
            event.type == SDL_EVENT_KEY_DOWN ? InputState::pressed : InputState::released;
        // Always return input pause and cancel events for android back button.
        android_input_events.push_back(
            game_event_types::Input{InputVerb::pause, input_state, default_touch_source_}
        );
        android_input_events.push_back(
            game_event_types::Input{InputVerb::cancel, input_state, default_touch_source_}
        );
        return android_input_events;
    }
    if (listen_for_valid_keyboard && event.type == SDL_EVENT_KEY_DOWN) {
        bool add_result = add_player_source(default_keyboard_source_);
        if (add_result) {
            return {};
        }
    }
    std::vector<game_event_types::Input> input_events;
    std::vector<InputVerbInfo> verbs = get_verbs_from_scancode(event.scancode);
    if (verbs.empty()) {
        return {};
    }
    for (size_t i = 0; i < verbs.size(); i++) {
        auto& amount_pressed = keyboard_verbs_pressed_[static_cast<size_t>(verbs[i].verb)];
        if (event.type == SDL_EVENT_KEY_DOWN && !event.repeat) {
            amount_pressed++;
        } else if (event.type == SDL_EVENT_KEY_UP) {
            if (amount_pressed == 0) {
                continue;
            }
            amount_pressed--;
        }
        if (amount_pressed == 0) {
            input_events.push_back(
                game_event_types::Input{
                    verbs[i].verb, InputState::released, default_keyboard_source_
                } // Not using event.which bc its somewhat unrekable and who
                  // needs to use multiple keyboards at once anyways
            );
        }
        if (event.type == SDL_EVENT_KEY_DOWN && amount_pressed >= 1 &&
            (verbs[i].activate_on_repeat || !event.repeat)) {
            input_events.push_back(
                game_event_types::Input{
                    verbs[i].verb, InputState::pressed, default_keyboard_source_
                }
            );
        }
    }
    return input_events;
}

std::vector<game_event_types::Input>
InputManager::handle_mouse_wheel_event(SDL_MouseWheelEvent& event) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return {};
    }
    if (event.integer_y > 0) {
        return {
            game_event_types::Input{InputVerb::zoom_in, InputState::pressed, default_mouse_source_}
        };
    } else if (event.integer_y < 0) {
        return {
            game_event_types::Input{InputVerb::zoom_out, InputState::pressed, default_mouse_source_}
        };
    }
    return {};
}

std::vector<game_event_types::Input>
InputManager::handle_gamepad_button_event(SDL_GamepadButtonEvent& event) {
    if (listen_for_new_gamepad && event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        InputSource event_source{InputType::controller, event.which};
        bool add_result = add_player_source(event_source);
        if (add_result) {
            return {};
        }
    }
    std::vector<InputVerb> verbs =
        get_verbs_from_gamepad_button(static_cast<SDL_GamepadButton>(event.button));
    if (verbs.empty()) {
        return {};
    }
    std::vector<game_event_types::Input> input_events;
    auto& amount_pressed_arr = gamepads_verbs_pressed_[event.which];
    for (size_t i = 0; i < verbs.size(); i++) {
        auto& amount_pressed = amount_pressed_arr[static_cast<size_t>(verbs[i])];
        if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
            amount_pressed++;
        } else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
            amount_pressed--;
        }
        if (amount_pressed == 0) {
            input_events.push_back(
                game_event_types::Input{
                    verbs[i], InputState::released, InputSource{InputType::controller, event.which}
                }
            );
        } else if (amount_pressed >= 1 && event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
            input_events.push_back(
                game_event_types::Input{
                    verbs[i], InputState::pressed, InputSource{InputType::controller, event.which}
                }
            );
        }
    }
    return input_events;
}

std::vector<game_event_types::Input>
InputManager::get_input_events_from_sdl_event(SDL_Event& event) {
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        return handle_keyboard_event(event.key);
    case SDL_EVENT_MOUSE_WHEEL:
        return handle_mouse_wheel_event(event.wheel);
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
        return handle_gamepad_button_event(event.gbutton);
    default:
        return {};
    }
}

void InputManager::handle_pinch_event(SDL_PinchFingerEvent& event) {
    if (event.type == SDL_EVENT_PINCH_UPDATE) {
        float scale_incrementor = event.scale - 1.f;
        game_events::push(game_event_types::ChangeLevelZoom{scale_incrementor, false});
    }
}
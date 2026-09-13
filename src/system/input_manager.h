#pragma once
#include "platformer/game_events.h"
#include <SDL3/SDL.h>
#include <array>
#include <optional>
#include <unordered_map>
#include <vector>

inline constexpr size_t max_binds_per_verb = 3;

std::string input_verb_to_string(InputVerb verb);
std::string input_type_to_string(InputType type);

struct ScancodeInfo {
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    bool activate_on_repeat = false;
};

using ScancodeBindings = std::
    array<std::array<ScancodeInfo, max_binds_per_verb>, static_cast<size_t>(InputVerb::verb_count)>;

using GamepadBindings = std::array<
    std::array<SDL_GamepadButton, max_binds_per_verb>,
    static_cast<size_t>(InputVerb::verb_count)>;

inline constexpr size_t max_player_sources = 4;
using PlayerSources = std::array<std::optional<InputSource>, max_player_sources>;

class InputManager {
  private:
    ScancodeBindings scancode_bindings_ = {};
    GamepadBindings gamepad_bindings_ = {};
    // Arrays of the amount a verb is pressed
    // e.g. two buttons bound to the same verb:
    // Prevents release event when one of the buttons is released and the other isnt.
    std::unordered_map<SDL_JoystickID, std::array<int, static_cast<size_t>(InputVerb::verb_count)>>
        gamepads_verbs_pressed_;
    std::array<int, static_cast<size_t>(InputVerb::verb_count)> keyboard_verbs_pressed_ = {};

    PlayerSources player_sources_ = {};
    size_t player_source_count_ = 0;

    const InputSource default_touch_source_ = InputSource{InputType::touch, 0};
    const InputSource default_keyboard_source_ = InputSource{InputType::keyboard, 0};
    const InputSource default_mouse_source_ = InputSource{InputType::mouse, 0};

    // Returns true if player source is added/removed
    bool add_player_source(const InputSource& source);
    bool remove_player_source(const InputSource& source);

    std::vector<game_event_types::Input> handle_keyboard_event(SDL_KeyboardEvent& event);
    std::vector<game_event_types::Input> handle_mouse_wheel_event(SDL_MouseWheelEvent& event);
    std::vector<game_event_types::Input> handle_gamepad_button_event(SDL_GamepadButtonEvent& event);

  public:
    InputManager();

    void
    bind_scancode_to_verb(InputVerb verb, ScancodeInfo binding, std::optional<size_t> at_index_opt);
    void unbind_scancode_from_verb(InputVerb verb, SDL_Scancode scancode);
    void clear_scancode_binding_at_index(InputVerb verb, size_t index);
    std::vector<InputVerbInfo> get_verbs_from_scancode(SDL_Scancode scancode);
    const std::array<ScancodeInfo, max_binds_per_verb>&
    get_scancodes_from_verb(InputVerb verb) const;
    const ScancodeBindings& get_scancode_bindings() const;

    void bind_gamepad_button_to_verb(
        InputVerb verb, SDL_GamepadButton button, std::optional<size_t> at_index_opt
    );
    void unbind_gamepad_button_from_verb(InputVerb verb, SDL_GamepadButton button);
    void clear_gamepad_button_binding_at_index(InputVerb verb, size_t index);
    std::vector<InputVerb> get_verbs_from_gamepad_button(SDL_GamepadButton button);
    const std::array<SDL_GamepadButton, max_binds_per_verb>&
    get_gamepad_buttons_from_verb(InputVerb verb) const;
    const GamepadBindings& get_gamepad_bindings() const;

    const PlayerSources& get_player_sources() const;
    size_t get_player_source_count() const;
    int sdl_gamepads_detected() const;

    std::string get_source_name(const InputSource& source);
    std::string get_gamepad_name(const SDL_JoystickID id);

    bool listen_for_new_gamepad = false;
    bool listen_for_valid_keyboard = false;

    void handle_gamepad_removed(SDL_GamepadDeviceEvent& event);

    // returns true if touch player is enabled/disabled.
    // only one touch player allowed on device at a time.
    bool enable_touch_player();
    bool disable_touch_player();
    void remove_player_source_at_index(size_t index);
    bool has_touch_screen();
    bool is_touch_player_enabled(size_t* at_index);

    std::vector<game_event_types::Input> get_input_events_from_sdl_event(SDL_Event& event);

    // void listenForScancodeBinding(InputVerb forVerb, size_t atIndex);

    // void listenForGamepadBinding(InputVerb forVerb, size_t atIndex);

    void handle_pinch_event(SDL_PinchFingerEvent& event);

    const std::vector<DefaultScancodeBinding> default_verb_bindings = {
        {InputVerb::up, SDL_SCANCODE_UP},
        {InputVerb::down, SDL_SCANCODE_DOWN},
        {InputVerb::left, SDL_SCANCODE_LEFT},
        {InputVerb::right, SDL_SCANCODE_RIGHT},
        {InputVerb::up, SDL_SCANCODE_W},
        {InputVerb::down, SDL_SCANCODE_S},
        {InputVerb::left, SDL_SCANCODE_A},
        {InputVerb::right, SDL_SCANCODE_D},
        {InputVerb::jump, SDL_SCANCODE_UP},
        {InputVerb::jump, SDL_SCANCODE_W},
        {InputVerb::jump, SDL_SCANCODE_SPACE},
        {InputVerb::sprint, SDL_SCANCODE_LSHIFT},
        {InputVerb::sprint, SDL_SCANCODE_RSHIFT},
        {InputVerb::confirm, SDL_SCANCODE_RETURN},
        {InputVerb::cancel, SDL_SCANCODE_ESCAPE},
        {InputVerb::pause, SDL_SCANCODE_ESCAPE},
        {InputVerb::toggle_fullscreen, SDL_SCANCODE_F11},
        {InputVerb::respawn, SDL_SCANCODE_R},
        {InputVerb::zoom_in, SDL_SCANCODE_KP_PLUS, true},
        {InputVerb::zoom_out, SDL_SCANCODE_KP_MINUS, true},
        {InputVerb::zoom_in, SDL_SCANCODE_EQUALS, true},
        {InputVerb::zoom_out, SDL_SCANCODE_MINUS, true},
        {InputVerb::zoom_reset, SDL_SCANCODE_KP_0},
        {InputVerb::zoom_reset, SDL_SCANCODE_0},
        {InputVerb::toggle_debug, SDL_SCANCODE_F3},
        {InputVerb::show_hitboxes, SDL_SCANCODE_F1}
    };

    const std::vector<DefaultButtonBinding> default_gamepad_bindings = {
        {InputVerb::up, SDL_GAMEPAD_BUTTON_DPAD_UP},
        {InputVerb::down, SDL_GAMEPAD_BUTTON_DPAD_DOWN},
        {InputVerb::left, SDL_GAMEPAD_BUTTON_DPAD_LEFT},
        {InputVerb::right, SDL_GAMEPAD_BUTTON_DPAD_RIGHT},
        {InputVerb::jump, SDL_GAMEPAD_BUTTON_EAST},
        {InputVerb::jump, SDL_GAMEPAD_BUTTON_SOUTH},
        {InputVerb::confirm, SDL_GAMEPAD_BUTTON_EAST},
        {InputVerb::pause, SDL_GAMEPAD_BUTTON_START},
        {InputVerb::pause, SDL_GAMEPAD_BUTTON_GUIDE},
        {InputVerb::cancel, SDL_GAMEPAD_BUTTON_SOUTH},
        {InputVerb::sprint, SDL_GAMEPAD_BUTTON_WEST},
        {InputVerb::sprint, SDL_GAMEPAD_BUTTON_NORTH},
    };
};
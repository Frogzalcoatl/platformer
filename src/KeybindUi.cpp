#include "KeybindUi.hpp"
#include <imgui.h>
#include <memory>

static const std::string inputSeparator = " / ";

static std::string scancodeBindsStr = "";
static std::string gButtonBindsStr = "";

void updateKeybindsUi(InputManager& input) {
    scancodeBindsStr.clear();
    gButtonBindsStr.clear();
    for (size_t i = 0; i < static_cast<int>(InputVerb::VerbCount); i++) {
        scancodeBindsStr += "\n" + inputVerbToString(static_cast<InputVerb>(i)) + ": ";
        auto scancodes = input.getScancodesFromVerb(static_cast<InputVerb>(i));
        bool addedAny = false;
        for (size_t j = 0; j < scancodes.max_size(); j++) {
            if (scancodes[j].scancode == SDL_SCANCODE_UNKNOWN) {
                continue;
            }
            scancodeBindsStr += SDL_GetScancodeName(scancodes[j].scancode);
            scancodeBindsStr += inputSeparator;
            addedAny = true;
        }
        if (addedAny) {
            // remove last separator
            scancodeBindsStr.erase(scancodeBindsStr.length() - inputSeparator.length());
        }
        gButtonBindsStr += "\n" + inputVerbToString(static_cast<InputVerb>(i)) + ": ";
        auto gButtons = input.getGamepadButtonsFromVerb(static_cast<InputVerb>(i));
        addedAny = false;
        for (size_t j = 0; j < gButtons.max_size(); j++) {
            if (gButtons[j] == SDL_GAMEPAD_BUTTON_INVALID) {
                continue;
            }
            gButtonBindsStr += SDL_GetGamepadStringForButton(gButtons[j]);
            gButtonBindsStr += inputSeparator;
            addedAny = true;
        }
        if (addedAny) {
            // remove last separator
            gButtonBindsStr.erase(gButtonBindsStr.length() - inputSeparator.length());
        }
    }
}

static const char KeyboardTabLabel[] = "Keyboard";
static const char ControllerTabLabel[] = "Controller";

void showKeybindUi() {
    ImGui::Begin("Controls");
    ImGui::BeginTabBar("ControlsTabBar");
    bool keyboardTab = ImGui::BeginTabItem(KeyboardTabLabel);
    if (keyboardTab) {
        if (!scancodeBindsStr.empty()) {
            // Avoid any c str % formatting issues.
            ImGui::TextUnformatted(scancodeBindsStr.c_str());
        }
        ImGui::EndTabItem();
    }
    bool controllerTab = ImGui::BeginTabItem(ControllerTabLabel);
    if (controllerTab) {
        if (!gButtonBindsStr.empty()) {
            ImGui::TextUnformatted(gButtonBindsStr.c_str());
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
}
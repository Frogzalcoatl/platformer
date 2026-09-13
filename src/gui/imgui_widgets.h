#pragma once
#include <imgui.h>

namespace imgui_widgets {
bool custom_arrow_button(const char* str_id, ImGuiDir dir, ImVec2 size);
bool custom_pause_button(const char* str_id, ImVec2 size);
}
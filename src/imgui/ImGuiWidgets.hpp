#pragma once
#include <imgui.h>

namespace Drawing {
bool CustomArrowButton(const char* str_id, ImGuiDir dir, ImVec2 size);
bool CustomPauseButton(const char* str_id, ImVec2 size);
}
#include "gui/imgui_widgets.h"
#include <SDL3/SDL.h>

bool imgui_widgets::custom_arrow_button(const char* str_id, ImGuiDir dir, ImVec2 size) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{0.f, 0.f});
    bool pressed = ImGui::Button(str_id, size);
    ImGui::PopStyleVar();
    ImVec2 pos_min = ImGui::GetItemRectMin();
    float arrow_size = SDL_min(size.x, size.y) * 0.5f;
    ImVec2 center = ImVec2{pos_min.x + size.x * 0.5f, pos_min.y + size.y * 0.5f};
    ImVec2 a, b, c;
    float radius = arrow_size * 0.5f;
    if (dir == ImGuiDir_Up) {
        a = ImVec2(center.x, center.y - radius);
        b = ImVec2(center.x - radius, center.y + radius);
        c = ImVec2(center.x + radius, center.y + radius);
    } else if (dir == ImGuiDir_Down) {
        a = ImVec2(center.x, center.y + radius);
        b = ImVec2(center.x - radius, center.y - radius);
        c = ImVec2(center.x + radius, center.y - radius);
    } else if (dir == ImGuiDir_Left) {
        a = ImVec2(center.x - radius, center.y);
        b = ImVec2(center.x + radius, center.y - radius);
        c = ImVec2(center.x + radius, center.y + radius);
    } else if (dir == ImGuiDir_Right) {
        a = ImVec2(center.x + radius, center.y);
        b = ImVec2(center.x - radius, center.y - radius);
        c = ImVec2(center.x - radius, center.y + radius);
    }
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (draw_list) {
        ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
        draw_list->AddTriangleFilled(a, b, c, color);
    }
    return pressed;
}

bool imgui_widgets::custom_pause_button(const char* str_id, ImVec2 size) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{0.f, 0.f});
    bool pressed = ImGui::Button(str_id, size);
    ImGui::PopStyleVar();
    ImVec2 pos_min = ImGui::GetItemRectMin();
    ImVec2 padding{size.x * 0.2f, size.y * 0.1f};
    float rec_width = size.x * 0.2f;
    float rec_height = size.y * 0.8f;
    ImVec2 rec1_min, rec1_max;
    rec1_min.x = pos_min.x + padding.x;
    rec1_min.y = pos_min.y + padding.y;
    rec1_max.x = rec1_min.x + rec_width;
    rec1_max.y = rec1_min.y + rec_height;
    ImVec2 rec2_min, rec2_max;
    rec2_min.x = rec1_max.x + padding.x;
    rec2_min.y = rec1_min.y;
    rec2_max.x = rec2_min.x + rec_width;
    rec2_max.y = rec1_max.y;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (draw_list) {
        ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
        draw_list->AddRectFilled(rec1_min, rec1_max, color);
        draw_list->AddRectFilled(rec2_min, rec2_max, color);
    }
    return pressed;
}
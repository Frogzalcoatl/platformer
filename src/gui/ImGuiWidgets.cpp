#include "gui/ImGuiWidgets.hpp"
#include <SDL3/SDL.h>

bool ImGuiWidgets::CustomArrowButton(const char* str_id, ImGuiDir dir, ImVec2 size) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{0.f, 0.f}); // Remove padding
    bool pressed = ImGui::Button(str_id, size);
    ImGui::PopStyleVar();
    ImVec2 posMin = ImGui::GetItemRectMin();
    float arrowSize = SDL_min(size.x, size.y) * 0.5f;
    ImVec2 center = ImVec2{posMin.x + size.x * 0.5f, posMin.y + size.y * 0.5f};
    ImVec2 a, b, c;
    float radius = arrowSize * 0.5f;
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
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    if (drawList) {
        ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
        drawList->AddTriangleFilled(a, b, c, color);
    }
    return pressed;
}

bool ImGuiWidgets::CustomPauseButton(const char* str_id, ImVec2 size) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{0.f, 0.f});
    bool pressed = ImGui::Button(str_id, size);
    ImGui::PopStyleVar();
    ImVec2 posMin = ImGui::GetItemRectMin();
    ImVec2 padding{size.x * 0.2f, size.y * 0.1f};
    float recWidth = size.x * 0.2f;
    float recHeight = size.y * 0.8f;
    ImVec2 rec1Min, rec1Max;
    rec1Min.x = posMin.x + padding.x;
    rec1Min.y = posMin.y + padding.y;
    rec1Max.x = rec1Min.x + recWidth;
    rec1Max.y = rec1Min.y + recHeight;
    ImVec2 rec2Min, rec2Max;
    rec2Min.x = rec1Max.x + padding.x;
    rec2Min.y = rec1Min.y;
    rec2Max.x = rec2Min.x + recWidth;
    rec2Max.y = rec1Max.y;
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    if (drawList) {
        ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
        drawList->AddRectFilled(rec1Min, rec1Max, color);
        drawList->AddRectFilled(rec2Min, rec2Max, color);
    }
    return pressed;
}
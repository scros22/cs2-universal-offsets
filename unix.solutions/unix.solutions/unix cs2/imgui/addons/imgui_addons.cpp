//============ Copyright KiwiHax, All rights reserved ============//
//
//  Purpose: 
//
//================================================================//

#include "../imgui_internal.h"
#include "imgui_addons.h"

#include <map>
#include <string>

using namespace ImGui;

void ImAdd::SeparatorText(const char* label, float thickness)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(ImVec2(-0.1f, g.FontSize), label_size.x, g.FontSize);

    const ImRect total_bb(pos, pos + size);
    ItemSize(total_bb);
    if (!ItemAdd(total_bb, id)) {
        return;
    }

    window->DrawList->AddText(pos, GetColorU32(ImGuiCol_TextDisabled), label);

    if (thickness > 0)
        window->DrawList->AddLine(pos + ImVec2(label_size.x + style.ItemInnerSpacing.x, size.y / 2), pos + ImVec2(size.x, size.y / 2), GetColorU32(ImGuiCol_Border), thickness);
}

void ImAdd::VSeparator(float margin, float thickness)
{
    if (thickness <= 0)
        return;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(ImVec2(thickness, -0.1f), thickness, thickness);

    const ImRect bb(pos, pos + size);
    const ImRect bb_rect(pos + ImVec2(0, margin), pos + size - ImVec2(0, margin));

    ItemSize(ImVec2(thickness, 0.0f));
    if (!ItemAdd(bb, 0))
        return;

    window->DrawList->AddRectFilled(bb_rect.Min, bb_rect.Max, GetColorU32(ImGuiCol_Border));
}

bool ImAdd::CheckBox(const char* label, bool* checked)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const float square_sz = g.FontSize + style.CellPadding.y * 2.0f;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImVec2(square_sz + style.ItemInnerSpacing.x + label_size.x, square_sz);

    const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
    const ImRect total_bb(pos, pos + size);
    ItemSize(size);
    if (!ItemAdd(total_bb, id))
        return false;

    // Behaviors
    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

    if (pressed) *checked = !*checked;

    // Colors
    ImVec4 colFrame = GetStyleColorVec4(*checked ? ImGuiCol_SliderGrab : (hovered && held) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);

    // Animations
    struct stColors_State {
        ImColor Frame;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
    }

    it_anim->second.Frame.Value = ImLerp(it_anim->second.Frame.Value, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);

    RenderNavCursor(total_bb, id);

    if (*checked)
    {
        window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, it_anim->second.Frame, style.FrameRounding);
        window->DrawList->AddRectFilledMultiColorRounded(check_bb.Min, check_bb.Max, GetColorU32(ImGuiCol_FrameBgShadow, 0.0f), GetColorU32(ImGuiCol_FrameBgShadow, 0.0f), GetColorU32(ImGuiCol_FrameBgShadow), GetColorU32(ImGuiCol_FrameBgShadow), style.FrameRounding);
    }
    else
    {
        window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, it_anim->second.Frame, style.FrameRounding);
        window->DrawList->AddRectFilledMultiColorRounded(check_bb.Min, check_bb.Max, IM_COL32(32, 32, 32, 255), IM_COL32(32, 32, 32, 255), IM_COL32(24, 24, 24, 255), IM_COL32(24, 24, 24, 255), style.FrameRounding);
    }


    if (style.FrameBorderSize > 0)
    {
        window->DrawList->AddRect(check_bb.Min, check_bb.Max, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding, 0, style.FrameBorderSize);
        window->DrawList->AddRect(check_bb.Min + ImVec2(style.FrameBorderSize, style.FrameBorderSize), check_bb.Max - ImVec2(style.FrameBorderSize, style.FrameBorderSize), IM_COL32(44, 44, 44, 255), style.FrameRounding, 0, style.FrameBorderSize);
    }


    DrawLabelShadow(window->DrawList, pos + ImVec2(size.y + style.ItemInnerSpacing.x, style.CellPadding.y), GetColorU32(ImGuiCol_Text), label);

    return pressed;
}

bool ImAdd::SelectableLabel(const char* label, bool selected, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, label_size.x, label_size.y);

    const ImRect total_bb(pos, pos + size);
    ItemSize(size);
    if (!ItemAdd(total_bb, id))
        return false;

    // Behaviors
    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

    // Colors
    ImVec4 colLabel = GetStyleColorVec4(selected ? ImGuiCol_SliderGrab : hovered ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    // Animations
    struct stColors_State {
        ImColor Label;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Label = colLabel;
    }

    it_anim->second.Label.Value = ImLerp(it_anim->second.Label.Value, colLabel, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);

    RenderNavCursor(total_bb, id);

    window->DrawList->AddText(pos + ImVec2(1.0f, 0.0f), it_anim->second.Label, label);

    return pressed;
}

bool ImAdd::SelectableFrame(const char* label, bool selected, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect total_bb(pos, pos + size);
    ItemSize(size);
    if (!ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

    ImVec4 colLabel;

    if (selected)
        colLabel = GetStyleColorVec4(ImGuiCol_SliderGrab);
    else if (hovered)
        colLabel = GetStyleColorVec4(ImGuiCol_SliderGrab);
    else
        colLabel = GetStyleColorVec4(ImGuiCol_TextDisabled);
    ImVec4 colFrame = GetStyleColorVec4(selected ? ImGuiCol_SliderGrab : ImGuiCol_Separator);

    ImVec4 colGlowMain = GetStyleColorVec4(ImGuiCol_SliderGrabActive);
    ImVec4 colGlowNull = colGlowMain;
    colGlowNull.w = 0.0f;
    ImVec4 colGlow = selected ? colGlowMain : colGlowNull;

    struct stColors_State {
        ImColor Label;
        ImColor Frame;
        ImColor Glow;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Label = colLabel;
        it_anim->second.Frame = colFrame;
        it_anim->second.Glow = colGlow;
    }

    it_anim->second.Label.Value = ImLerp(it_anim->second.Label.Value, colLabel, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);
    it_anim->second.Frame.Value = ImLerp(it_anim->second.Frame.Value, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);
    it_anim->second.Glow.Value = ImLerp(it_anim->second.Glow.Value, colGlow, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);

    RenderNavCursor(total_bb, id);

    DrawLabelShadow(window->DrawList, pos + ImTrunc(size / 2 - label_size / 2), it_anim->second.Label, label);

    return pressed;
}

bool ImAdd::Button(const char* label, const ImVec2& size_arg, ImDrawFlags draw_flags, ImGuiButtonFlags button_flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect total_bb(pos, pos + size);
    ItemSize(size);
    if (!ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

    ImVec4 colFrame = GetStyleColorVec4((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);

    struct stColors_State {
        ImColor Frame;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
    }

    it_anim->second.Frame.Value = ImLerp(it_anim->second.Frame.Value, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);

    RenderNavCursor(total_bb, id);

    window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, GetColorU32(it_anim->second.Frame, style.Alpha), style.FrameRounding, draw_flags);
    window->DrawList->AddRectFilledMultiColorRounded(total_bb.Min, total_bb.Max, GetColorU32(ImGuiCol_ButtonShadow, 0.0f), GetColorU32(ImGuiCol_ButtonShadow, 0.0f), GetColorU32(ImGuiCol_ButtonShadow), GetColorU32(ImGuiCol_ButtonShadow), style.FrameRounding, draw_flags);

    if (style.FrameBorderSize > 0)
    {
        window->DrawList->AddRect(total_bb.Min, total_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding, draw_flags, style.FrameBorderSize);
    }

    RenderTextClipped(total_bb.Min + style.FramePadding, total_bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &total_bb);

    return pressed;
}

bool ImAdd::SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const float width = CalcItemWidth();
    const ImVec2 pos = window->DC.CursorPos;

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const bool has_label = label_size.x > 0;
    const float frame_pos_y = has_label ? (g.FontSize + style.ItemInnerSpacing.y) : 0.0f;
    const float frame_height = style.GrabMinSize;

    const ImRect frame_bb(pos + ImVec2(0, frame_pos_y - 3.0f), pos + ImVec2(width, frame_pos_y + frame_height + 3.0f));

    const ImRect total_bb(pos, frame_bb.Max);

    ItemSize(total_bb);
    if (!ItemAdd(total_bb, id, &frame_bb, 0))
        return false;

    if (format == NULL)
        format = DataTypeGetInfo(data_type)->PrintFmt;

    const bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.ItemFlags);
    const bool clicked = hovered && IsMouseClicked(0, ImGuiInputFlags_None, id);
    const bool held = g.ActiveId == id;
    const bool make_active = (clicked || g.NavActivateId == id);

    if (make_active)
    {
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    // Colors
    ImVec4 colFrame = GetStyleColorVec4((hovered && held) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    ImVec4 colLine = GetStyleColorVec4(held ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab);

    // Animations
    struct stColors_State {
        ImColor Frame;
        ImColor Line;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
        it_anim->second.Line = colLine;
    }

    it_anim->second.Frame.Value = ImLerp(it_anim->second.Frame.Value, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);
    it_anim->second.Line.Value = ImLerp(it_anim->second.Line.Value, colLine, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);

    // Grab logic
    ImRect grab_bb;
    const bool value_changed = SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, 0, &grab_bb);
    if (value_changed)
        MarkItemEdited(id);

    float relative_value = 0.0f;
    if (data_type == ImGuiDataType_Float)
    {
        float val = *(float*)p_data;
        float min_val = *(float*)p_min;
        float max_val = *(float*)p_max;
        relative_value = (val - min_val) / (max_val - min_val);
    }
    else if (data_type == ImGuiDataType_S32)
    {
        int val = *(int*)p_data;
        int min_val = *(int*)p_min;
        int max_val = *(int*)p_max;
        relative_value = (float)(val - min_val) / (float)(max_val - min_val);
    }

    relative_value = ImClamp(relative_value, 0.0f, 1.0f);

    const float pad = ImTrunc(frame_height / 3.0f);

    window->DrawList->AddRectFilledMultiColor(frame_bb.Min, frame_bb.Max, IM_COL32(34, 34, 34, 255), IM_COL32(34, 34, 34, 255), IM_COL32(24, 24, 24, 255), IM_COL32(24, 24, 24, 255));

    ImVec2 fill_end = ImTrunc(ImVec2(frame_bb.Min.x + relative_value * frame_bb.GetWidth(), frame_bb.Max.y));

    ImRect slider_bb = ImRect(frame_bb.Min, fill_end);

    if (slider_bb.Max.x > slider_bb.Min.x)
    {
        window->DrawList->AddRectFilled(slider_bb.Min, slider_bb.Max, it_anim->second.Line, style.FrameRounding);
        window->DrawList->AddRectFilledMultiColorRounded(slider_bb.Min, slider_bb.Max, GetColorU32(ImGuiCol_FrameBgShadow, 0.0f), GetColorU32(ImGuiCol_FrameBgShadow, 0.0f), GetColorU32(ImGuiCol_FrameBgShadow), GetColorU32(ImGuiCol_FrameBgShadow), style.FrameRounding);

        if (style.FrameBorderSize > 0)
        {
            window->DrawList->AddLine(ImVec2(slider_bb.Max.x - style.FrameBorderSize, slider_bb.Min.y + style.FrameBorderSize), ImVec2(slider_bb.Max.x - style.FrameBorderSize, slider_bb.Min.y + slider_bb.GetHeight() - style.FrameBorderSize), GetColorU32(ImGuiCol_Border), style.FrameBorderSize);
        }
    }

    if (style.FrameBorderSize > 0)
    {
        window->DrawList->AddRect(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding, 0, style.FrameBorderSize);
        window->DrawList->AddRect(frame_bb.Min + ImVec2(1, 1), frame_bb.Max - ImVec2(1, 1), IM_COL32(44, 44, 44, 255), style.FrameRounding, 0, style.FrameBorderSize);
    }

    char value_buf[64];
    const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

    if (has_label) {
        DrawLabelShadow(window->DrawList, total_bb.Min, GetColorU32(ImGuiCol_Text), label);
    }

    ImVec2 text_size = CalcTextSize(value_buf);
    ImVec2 text_pos = ImVec2(frame_bb.Min.x + (frame_bb.GetWidth() - text_size.x) * 0.5f, frame_bb.Min.y + (frame_bb.GetHeight() - text_size.y) * 0.5f);

    DrawLabelShadow(window->DrawList, text_pos, IM_COL32(255, 255, 255, 255), value_buf);


    window->DrawList->AddText(text_pos, IM_COL32(255, 255, 255, 255), value_buf);


    return value_changed;
}

bool ImAdd::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format)
{
    return ImAdd::SliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format);
}

bool ImAdd::SliderInt(const char* label, int* v, int v_min, int v_max, const char* format)
{
    return ImAdd::SliderScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format);
}

bool ImAdd::DragScalar(const char* label, ImGuiDataType data_type, void* p_data, float v_speed, const void* p_min, const void* p_max, const char* format)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    std::string hidden_label = "##";
    hidden_label += label;

    BeginGroup();
    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.ItemInnerSpacing.y));
    Text(label);
    PopStyleVar();

    // Custom frame
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImVec2(CalcItemWidth(), GetFrameHeight());

    window->DrawList->AddRectFilled(pos, pos + size, GetColorU32(ImGuiCol_Button), style.FrameRounding);
    if (style.FrameBorderSize > 0)
    {
        window->DrawList->AddRect(pos + ImVec2(style.FrameBorderSize, style.FrameBorderSize), pos + size - ImVec2(style.FrameBorderSize, style.FrameBorderSize), GetColorU32(ImGuiCol_Border), style.FrameRounding, 0, style.FrameBorderSize);
        window->DrawList->AddRect(pos, pos + size, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding, 0, style.FrameBorderSize);
    }

    PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4());
    PushStyleColor(ImGuiCol_FrameBgActive, ImVec4());
    PushStyleColor(ImGuiCol_Border, ImVec4());
    PushStyleColor(ImGuiCol_BorderShadow, ImVec4());
    bool result = ImGui::DragScalar(hidden_label.c_str(), data_type, p_data, v_speed, p_min, p_max, format);
    PopStyleColor(5);
    EndGroup();

    return result;
}

bool ImAdd::DragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format)
{
    return ImAdd::DragScalar(label, ImGuiDataType_Float, v, v_speed, &v_min, &v_max, format);
}

bool ImAdd::DragInt(const char* label, int* v, float v_speed, int v_min, int v_max, const char* format)
{
    return ImAdd::DragScalar(label, ImGuiDataType_S32, v, v_speed, &v_min, &v_max, format);
}

bool ImAdd::InputText(const char* label, const char* placeholder, char* buf, size_t buf_size, ImDrawFlags draw_flags, ImGuiInputTextFlags input_flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT(!(input_flags & ImGuiInputTextFlags_Multiline)); // call InputTextMultiline()

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = window->DC.CursorPos + ImVec2(0, g.FontSize + style.ItemInnerSpacing.y);
    ImVec2 size = ImVec2(CalcItemWidth(), GetFrameHeight());

    window->DrawList->AddRectFilled(pos, pos + size, GetColorU32(ImGuiCol_FrameBg), style.FrameRounding, draw_flags);

    if (style.FrameBorderSize > 0)
    {
        window->DrawList->AddRect(pos, pos + size, GetColorU32(ImGuiCol_Border), style.FrameRounding, draw_flags, style.FrameBorderSize);
    }

    BeginGroup();
    PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
    PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));
    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.ItemInnerSpacing.y));
    Text(placeholder);
    PopStyleVar();
    bool result = ImGui::InputTextEx(std::string("##" + std::string(label)).c_str(), NULL, buf, (int)buf_size, ImVec2(0, 0), input_flags, callback, user_data);
    PopStyleColor(3);
    EndGroup();

    if (strlen(buf) == 0 && !IsItemActive())
    {
        window->DrawList->AddText(pos + style.FramePadding + ImVec2(style.FrameBorderSize, style.FrameBorderSize), IM_COL32_BLACK, placeholder);
        window->DrawList->AddText(pos + style.FramePadding, GetColorU32(ImGuiCol_TextDisabled), placeholder);
    }

    return result;
}

bool ImAdd::Combo(const char* label, int* selected_index, std::vector<const char*> items)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const float square_sz = g.FontSize + style.FramePadding.y * 2.0f;

    const float width = CalcItemWidth();
    const float height = GetFrameHeight();

    int items_count = items.size();

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImVec2(width, height + (label_size.x > 0.0f ? (label_size.y + style.ItemInnerSpacing.y) : 0.0f));

    const ImRect frame_bb(pos + ImVec2(0.0f, label_size.x > 0.0f ? (label_size.y + style.ItemInnerSpacing.y) : 0.0f), pos + size);
    const ImRect total_bb(pos, pos + size);
    ItemSize(size);
    if (!ItemAdd(total_bb, id))
        return false;

    // Behaviors
    bool hovered, held;
    bool pressed = ButtonBehavior(frame_bb, id, &hovered, &held);

    std::string popup_str_id = std::string(std::string(label) + "::combo_popup");

    if (pressed)
    {
        OpenPopup(popup_str_id.c_str());
    }

    PushStyleVar(ImGuiStyleVar_WindowPadding, style.FramePadding);
    PushStyleVar(ImGuiStyleVar_ItemSpacing, style.FramePadding);
    if (BeginPopupEx(GetID(popup_str_id.c_str()), ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove))
    {

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pmin = ImGui::GetWindowPos();
        ImVec2 pmax = pmin + ImGui::GetWindowSize();

        dl->AddRect(pmin, pmax, IM_COL32(0, 0, 0, 255), style.FrameRounding);
        dl->AddRect(pmin + ImVec2(1, 1), pmax - ImVec2(1, 1), IM_COL32(44, 44, 44, 255), style.FrameRounding);
        dl->AddRectFilledMultiColor(pmin + ImVec2(2, 2), pmax - ImVec2(2, 2), IM_COL32(34, 34, 34, 255), IM_COL32(34, 34, 34, 255), IM_COL32(24, 24, 24, 255), IM_COL32(24, 24, 24, 255));

        SetWindowPos(frame_bb.Min + ImVec2(0, height + style.FramePadding.y), ImGuiCond_Always);
        SetWindowSize(ImVec2(width, ImGui::GetFontSize() * items_count + style.FramePadding.y * (items_count + 1)), ImGuiCond_Always);

        PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        for (int i = 0; i < items_count; i++)
        {
            if (ImAdd::SelectableLabel(items[i], i == *selected_index, ImVec2(-0.1f, GetFontSize())))
            {
                *selected_index = i;
                CloseCurrentPopup();
            }
        }
        PopStyleVar();

        EndPopup();
    }
    PopStyleVar(2);

    // Colors
    ImVec4 colFrame = GetStyleColorVec4((hovered && held) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);

    // Animations
    struct stColors_State {
        ImColor Frame;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
    }

    it_anim->second.Frame.Value = ImLerp(it_anim->second.Frame.Value, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);

    RenderNavCursor(frame_bb, id);

    DrawLabelShadow(window->DrawList, pos, GetColorU32(ImGuiCol_Text), label);

    window->DrawList->AddRectFilledMultiColor(frame_bb.Min, frame_bb.Max, IM_COL32(34, 34, 34, 255), IM_COL32(34, 34, 34, 255), IM_COL32(24, 24, 24, 255), IM_COL32(24, 24, 24, 255));


    if (style.FrameBorderSize > 0)
    {

        window->DrawList->AddRect(frame_bb.Min, frame_bb.Max, IM_COL32(0, 0, 0, 255), style.FrameRounding, 0, 1.0f);
        window->DrawList->AddRect(frame_bb.Min + ImVec2(1, 1), frame_bb.Max - ImVec2(1, 1), IM_COL32(44, 44, 44, 255), style.FrameRounding, 0, 1.0f);
        window->DrawList->AddLine(ImVec2(frame_bb.Max.x - height, frame_bb.Min.y + 1.0f), ImVec2(frame_bb.Max.x - height, frame_bb.Max.y - 1.0f), IM_COL32(44, 44, 44, 255), 1.0f);
    }


    std::string preview_item;
    if (*selected_index > items.size()) {
        preview_item = "*unknown item*";
    }
    else
    {
        preview_item = items[*selected_index];
    }

    DrawLabelShadow(window->DrawList, frame_bb.Min + style.FramePadding + ImVec2(1.0f, 0.0f), GetColorU32(ImGuiCol_Text), preview_item.c_str());

    for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++) if (dx != 0 || dy != 0) window->DrawList->AddText(ImVec2(frame_bb.Max.x - height * 0.5f - ImGui::CalcTextSize(IsPopupOpen(popup_str_id.c_str()) ? "-" : "+").x * 0.5f, frame_bb.Min.y + height * 0.5f - ImGui::CalcTextSize(IsPopupOpen(popup_str_id.c_str()) ? "-" : "+").y * 0.5f - 1.0f) + ImVec2((float)dx, (float)dy), IM_COL32(0, 0, 0, 255), IsPopupOpen(popup_str_id.c_str()) ? "-" : "+"); window->DrawList->AddText(ImVec2(frame_bb.Max.x - height * 0.5f - ImGui::CalcTextSize(IsPopupOpen(popup_str_id.c_str()) ? "-" : "+").x * 0.5f, frame_bb.Min.y + height * 0.5f - ImGui::CalcTextSize(IsPopupOpen(popup_str_id.c_str()) ? "-" : "+").y * 0.5f - 1.0f), GetColorU32(ImGuiCol_Text), IsPopupOpen(popup_str_id.c_str()) ? "-" : "+");

    return pressed;
}

bool ImAdd::SliderAlpha(const char* str_id, ImVec4& col)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImGuiIO& io = g.IO;
    const ImGuiID id = window->GetID(str_id);

    float width = CalcItemWidth();
    float height = GetFrameHeight();
    float half_height = ImTrunc(height / 2.0f);

    ImVec2 pos = window->DC.CursorPos;
    const ImVec2 size(width, height);

    const ImRect bb(pos, pos + size);

    ItemSize(bb);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    const float slider_min_x = bb.Min.x + half_height;
    const float slider_max_x = bb.Max.x - half_height;

    if (held)
    {
        // Calculate the available slider area
        const float slider_width = slider_max_x - slider_min_x;

        if (slider_width > 0.0f)
        {
            // Clamp mouse position to slider bounds and calculate alpha value
            float mouse_x = ImClamp(io.MousePos.x, slider_min_x, slider_max_x);
            col.w = 1.0f - ((mouse_x - slider_min_x) / slider_width); // Inverted for correct alpha mapping
            col.w = ImClamp(col.w, 0.0f, 1.0f);
        }
    }

    ImColor col_rgb = ImVec4(col.x, col.y, col.z, 1.0f);
    ImColor col_rgb_width_alpha = ImVec4(col.x, col.y, col.z, col.w);

    window->DrawList->AddRectFilled(pos, pos + ImVec2(half_height, height), col_rgb, style.FrameRounding, ImDrawFlags_RoundCornersLeft);
    RenderColorRectWithAlphaCheckerboard(window->DrawList, pos + ImVec2(half_height, 0), pos + size, 0, half_height, ImVec2(0.0f, 0.0f), style.FrameRounding, ImDrawFlags_RoundCornersRight);
    window->DrawList->AddRectFilledMultiColorRounded(pos + ImVec2(half_height, 0), pos + ImVec2(width - half_height, height), col_rgb, col_rgb & ~IM_COL32_A_MASK, col_rgb & ~IM_COL32_A_MASK, col_rgb);

    // Draw border
    if (style.FrameBorderSize > 0)
    {
        window->DrawList->AddRect(pos, pos + size, GetColorU32(ImGuiCol_Border), style.FrameRounding, 0, style.FrameBorderSize);
    }

    // Calculate circle position within slider bounds (inverted for correct visual mapping)
    const float circle_radius = ImMax(ImTrunc(g.FontSize / 2.0f) - 2.0f, 1.0f);
    const float slider_width = slider_max_x - slider_min_x;
    float circle_x = slider_min_x + slider_width * (1.0f - col.w); // Inverted for correct visual position

    RenderColorRectWithAlphaCheckerboard(window->DrawList, ImVec2(circle_x - circle_radius, pos.y + half_height - circle_radius), ImVec2(circle_x + circle_radius, pos.y + half_height + circle_radius), col_rgb_width_alpha, circle_radius, ImVec2(0.0f, 0.0f), circle_radius + 1.0f);
    window->DrawList->AddCircle(ImVec2(circle_x, pos.y + half_height), circle_radius + 1.0f, IM_COL32_BLACK, 0, 1.0f);

    return pressed;
}

bool ImAdd::SliderHue(const char* str_id, ImVec4& col)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImGuiIO& io = g.IO;
    const ImGuiID id = window->GetID(str_id);

    float width = CalcItemWidth();
    float height = GetFrameHeight();
    float half_height = ImTrunc(height / 2.0f);

    ImVec2 pos = window->DC.CursorPos;
    const ImVec2 size(width, height);

    const ImRect bb(pos, pos + size);

    ItemSize(bb);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    const float slider_min_x = bb.Min.x + half_height;
    const float slider_max_x = bb.Max.x - half_height;

    // Extract HSV values from RGB color
    float h, s, v;
    ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, h, s, v);

    if (held)
    {
        // Calculate the available slider area
        const float slider_width = slider_max_x - slider_min_x;

        if (slider_width > 0.0f)
        {
            // Clamp mouse position to slider bounds and calculate hue value
            float mouse_x = ImClamp(io.MousePos.x, slider_min_x, slider_max_x);
            h = (mouse_x - slider_min_x) / slider_width; // Hue ranges from 0.0 to 1.0
            //h = ImClamp(h, 0.0f, 1.0f);
            h = ImClamp(h, 0.0f, 0.996f); // HACK : prevent teleporting to start point (also red), 254 / 255

            // Convert back to RGB
            ImGui::ColorConvertHSVtoRGB(h, s, v, col.x, col.y, col.z);
        }
    }

    // Draw hue gradient in the middle section
    const ImVec2 gradient_start = pos + ImVec2(half_height, 0);
    const ImVec2 gradient_end = pos + ImVec2(width - half_height, height);

    // Draw hue spectrum gradient
    const int num_segments = 6; // For the 6 main hue segments
    const float segment_width = (width - height) / num_segments;

    for (int i = 0; i < num_segments; i++)
    {
        float hue_start = i / (float)num_segments;
        float hue_end = (i + 1) / (float)num_segments;

        ImVec4 color_start, color_end;
        ImGui::ColorConvertHSVtoRGB(hue_start, 1.0f, 1.0f, color_start.x, color_start.y, color_start.z);
        ImGui::ColorConvertHSVtoRGB(hue_end, 1.0f, 1.0f, color_end.x, color_end.y, color_end.z);
        color_start.w = color_end.w = 1.0f;

        ImVec2 seg_start = gradient_start + ImVec2(segment_width * i, 0);
        ImVec2 seg_end = gradient_start + ImVec2(segment_width * (i + 1), height);

        window->DrawList->AddRectFilledMultiColor(
            seg_start, seg_end,
            ImColor(color_start), ImColor(color_end), ImColor(color_end), ImColor(color_start)
        );
    }

    // Draw static color areas on left and right (repeating the hue spectrum edges)
    ImVec4 left_color, right_color;
    ImGui::ColorConvertHSVtoRGB(0.0f, 1.0f, 1.0f, left_color.x, left_color.y, left_color.z);
    ImGui::ColorConvertHSVtoRGB(1.0f, 1.0f, 1.0f, right_color.x, right_color.y, right_color.z);
    left_color.w = right_color.w = 1.0f;

    window->DrawList->AddRectFilled(pos, pos + ImVec2(half_height, height), ImColor(left_color), style.FrameRounding, ImDrawFlags_RoundCornersLeft);
    window->DrawList->AddRectFilled(pos + ImVec2(width - half_height - ImFmod(g.FontSize, 2.0f), 0), pos + size, ImColor(right_color), style.FrameRounding, ImDrawFlags_RoundCornersRight);

    // Draw borders
    if (style.FrameBorderSize > 0)
    {
        window->DrawList->AddRect(pos, pos + size, GetColorU32(ImGuiCol_Border), style.FrameRounding, 0, style.FrameBorderSize);
    }

    // Calculate circle position within slider bounds
    const float circle_radius = ImMax(ImTrunc(g.FontSize / 2.0f) - 2.0f, 1.0f);
    const float slider_width = slider_max_x - slider_min_x;
    float circle_x = slider_min_x + slider_width * h;

    // Het HUE color
    ImVec4 hue_color;
    ImGui::ColorConvertHSVtoRGB(h, 1.0f, 1.0f, hue_color.x, hue_color.y, hue_color.z); // Full saturation & value
    hue_color.w = 1.0f;

    window->DrawList->AddCircleFilled(ImVec2(circle_x, pos.y + half_height), circle_radius, GetColorU32(hue_color));
    window->DrawList->AddCircle(ImVec2(circle_x, pos.y + half_height), circle_radius + 1.0f, IM_COL32_BLACK, 0, 1.0f);

    return pressed;
}

bool ImAdd::ColorPalette(const char* str_id, ImVec4& col, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImGuiIO& io = g.IO;
    const ImGuiID id = window->GetID(str_id);

    // Use CalcItemWidth for both dimensions if not specified
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, CalcItemWidth(), CalcItemWidth());

    const ImRect bb(pos, pos + size);

    ItemSize(bb);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    // Extract HSV values from RGB color
    float h, s, v;
    ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, h, s, v);

    if (held || hovered)
    {
        if (held)
        {
            // Calculate saturation and value from mouse position
            s = ImClamp((io.MousePos.x - bb.Min.x) / size.x, 0.0f, 1.0f);
            //v = 1.0f - ImClamp((io.MousePos.y - bb.Min.y) / size.y, 0.0f, 1.0f); // Inverted Y axis (0 at bottom, 1 at top)
            v = 1.0f - ImClamp((io.MousePos.y - bb.Min.y) / size.y, 0.0f, 0.996f); // HACK : fix teleportation

            // Convert back to RGB
            ImGui::ColorConvertHSVtoRGB(h, s, v, col.x, col.y, col.z);
        }
    }

    // Get the hue color at full saturation and value
    ImVec4 hue_color;
    ImGui::ColorConvertHSVtoRGB(h, 1.0f, 1.0f, hue_color.x, hue_color.y, hue_color.z);
    hue_color.w = 1.0f;
    ImU32 hue_color32 = ImGui::ColorConvertFloat4ToU32(hue_color);

    // Draw the saturation-value gradient using ImGui's method
    // First rectangle: White to hue color (top to bottom)
    window->DrawList->AddRectFilledMultiColorRounded(bb.Min, bb.Max,
        IM_COL32_WHITE, hue_color32, hue_color32, IM_COL32_WHITE,
        style.FrameRounding + (style.FrameRounding > 0 ? 1.5f : 0.0f)
    );

    // Second rectangle: Transparent to black (left to right) overlayed on top
    window->DrawList->AddRectFilledMultiColorRounded(bb.Min, bb.Max,
        IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), IM_COL32_BLACK, IM_COL32_BLACK,
        style.FrameRounding
    );

    if (style.FrameBorderSize > 0)
    {
        window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_Border), style.FrameRounding, 0, style.FrameBorderSize);
    }

    // Draw the selection circle
    float circle_x = bb.Min.x + s * size.x;
    float circle_y = bb.Min.y + (1.0f - v) * size.y; // Inverted Y axis
    const float circle_radius = ImMax(ImTrunc(g.FontSize / 2.0f) - 2.0f, 1.0f);

    ImVec4 col_rgb_without_alpha(col.x, col.y, col.z, 1.0f);

    window->DrawList->AddCircleFilled(ImVec2(circle_x, circle_y), circle_radius, GetColorU32(col_rgb_without_alpha));
    window->DrawList->AddCircle(ImVec2(circle_x, circle_y), circle_radius + 1.0f, IM_COL32_BLACK, 0, 1.0f);
    window->DrawList->AddCircle(ImVec2(circle_x, circle_y), circle_radius, IM_COL32_WHITE, 0, 1.0f);

    return pressed;
}

bool ImAdd::ColorButton(const char* desc_id, const ImVec4& col, bool has_alpha, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(desc_id);

    ImVec2 pos = window->DC.CursorPos;
    const ImVec2 size(CalcItemSize(size_arg, g.FontSize, g.FontSize));
    const ImRect bb(pos, pos + size);

    ItemSize(bb);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    ImVec4 col_rgb = col;
    ImVec4 col_rgb_without_alpha(col_rgb.x, col_rgb.y, col_rgb.z, 1.0f);

    ImVec4 col_source = has_alpha ? col_rgb : col_rgb_without_alpha;

    if (col_source.w < 1.0f && has_alpha)
        RenderColorRectWithAlphaCheckerboard(window->DrawList, pos, pos + size, GetColorU32(col_source), size.y / 2, ImVec2(0, 0), style.FrameRounding);
    else
        window->DrawList->AddRectFilled(pos, pos + size, GetColorU32(col_source), style.FrameRounding);

    if (style.FrameBorderSize > 0)
    {
        window->DrawList->AddRect(pos, pos + size, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding, 0, style.FrameBorderSize);
        window->DrawList->AddRect(pos + ImVec2(style.FrameBorderSize, style.FrameBorderSize), pos + size - ImVec2(style.FrameBorderSize, style.FrameBorderSize), IM_COL32(44, 44, 44, 255), style.FrameRounding, 0, style.FrameBorderSize);
    }

    RenderNavCursor(bb, id);

    return pressed;
}

void ImAdd::ColorPicker4(const char* label, float col[4], bool has_alpha)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;

    ImVec4 col_v4(col[0], col[1], col[2], has_alpha ? col[3] : 1.0F);

    float width = 200;

    PushID(label);
    BeginGroup();

    std::string str_label = label;
    std::string str_colp_id = str_label + "::color_palette";
    std::string str_hue_id = str_label + "::hue";

    ImAdd::ColorPalette(str_colp_id.c_str(), col_v4, ImVec2(width, ImTrunc(width / 2)));

    PushItemWidth(width);
    PushStyleVar(ImGuiStyleVar_FramePadding, style.CellPadding);

    ImAdd::SliderHue(str_hue_id.c_str(), col_v4);

    if (has_alpha)
    {
        std::string str_alpha_id = str_label + "::alpha";
        ImAdd::SliderAlpha(str_alpha_id.c_str(), col_v4);

    }

    PopStyleVar();
    PopItemWidth();

    EndGroup();
    PopID();

    col[0] = col_v4.x;
    col[1] = col_v4.y;
    col[2] = col_v4.z;
    col[3] = col_v4.w;
}

bool ImAdd::ColorEdit4(const char* name, float col[4])
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImVec4 col_v4(col[0], col[1], col[2], col[3]);

    ImVec2 pos = window->DC.CursorPos;

    std::string popup_str_id = std::string(std::string(name) + "::color_edit_4");

    PushID(GetID(name));
    BeginGroup();

    float square_sz = GetColorPickerWidth();

    bool result = ImAdd::ColorButton(name, col_v4, true, ImVec2(square_sz, square_sz));

    EndGroup();
    PopID();

    if (result)
    {
        OpenPopup(popup_str_id.c_str());
    }

    PushStyleVar(ImGuiStyleVar_WindowPadding, style.FramePadding);
    PushStyleVar(ImGuiStyleVar_ItemSpacing, style.FramePadding);

    if (BeginPopupEx(GetID(popup_str_id.c_str()), ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        SetWindowPos(pos - style.WindowPadding);

        ImVec4 col_rgba = ImVec4(0, 0, 0, 0);
        col_rgba.x = col[0];
        col_rgba.y = col[1];
        col_rgba.z = col[2];
        col_rgba.w = col[3];

        ImAdd::ColorButton((popup_str_id + "::color_preview").c_str(), col_rgba, true, ImVec2(square_sz, square_sz));

        char hex_buf[64];
        int i[4] = { IM_F32_TO_INT8_UNBOUND(col[0]), IM_F32_TO_INT8_UNBOUND(col[1]), IM_F32_TO_INT8_UNBOUND(col[2]), IM_F32_TO_INT8_UNBOUND(col[3]) };
        ImFormatString(hex_buf, IM_ARRAYSIZE(hex_buf), "#%02X%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255), ImClamp(i[3], 0, 255));

        ImGui::SameLine();
        ImGui::SetCursorPosY(style.WindowPadding.y + style.CellPadding.y);
        ImGui::Text("%s", name);

        ImGui::SameLine(ImGui::CalcItemSize(ImVec2(-0.1f, 0), 0, 0).x - CalcTextSize(hex_buf).x + style.WindowPadding.x);
        ImGui::SetCursorPosY(style.WindowPadding.y + style.CellPadding.y);
        ImGui::TextDisabled("%s", hex_buf);

        ImAdd::ColorPicker4(name, col, true);

        EndPopup();
    }

    PopStyleVar(2);

    return result;
}

bool ImAdd::KeyBind(const char* str_id, ImGuiKey* k, const ImVec2& size_arg)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(str_id);

    ImVec2 pos = window->DC.CursorPos;

    char buf_display[32] = "[unbinded]";

    bool is_selecing = false;

    if (*k != 0 && g.ActiveId != id)
    {
        strcpy_s(buf_display, sizeof buf_display, std::string("[" + std::string(GetKeyName(*k)) + "]").c_str());
    }
    else if (g.ActiveId == id)
    {
        is_selecing = true;
        strcpy_s(buf_display, sizeof buf_display, "[...]");
    }

    ImVec2 size = CalcItemSize(size_arg, ImAdd::CalcKeyBindWidth(*k), g.FontSize + style.CellPadding.y * 2);
    ImRect frame_bb(pos, pos + size);
    ImRect total_bb(pos, frame_bb.Max);

    ImGui::ItemSize(total_bb);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    const bool hovered = ImGui::ItemHoverable(frame_bb, id, 0);

    // Colors
    ImVec4 colLabel = GetStyleColorVec4(is_selecing ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    // Animations
    struct stColors_State {
        ImColor Label;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Label = colLabel;
    }

    it_anim->second.Label.Value = ImLerp(it_anim->second.Label.Value, colLabel, 1.0f / IMADD_ANIMATIONS_SPEED * GetIO().DeltaTime);

    if (hovered)
    {
        ImGui::SetHoveredID(id);
        g.MouseCursor = ImGuiMouseCursor_Hand;
    }

    const bool user_clicked = hovered && IsMouseClicked(ImGuiMouseButton_Left);

    if (user_clicked)
    {
        /*
        if (g.ActiveId != id)
        {
            memset(io.MouseDown, 0, sizeof(io.MouseDown));
            //memset(io.KeysDown, 0, sizeof(io.KeysDown));
            *k = 0;
        }
        */
        ImGui::SetActiveID(id, window);
        ImGui::FocusWindow(window);
    }
    else if (IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
    }

    bool value_changed = false;
    int key = *k;

    if (hovered && IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (g.ActiveId != id)
        {
            // Start capturing
            memset(io.MouseDown, 0, sizeof(io.MouseDown));
            //memset(io.KeysData, {}, sizeof(io.KeysData));
            //memset(io.KeysDown, 0, sizeof(io.KeysDown));
            SetActiveID(id, window);
            FocusWindow(window);
        }
    }

    if (IsMouseClicked(ImGuiMouseButton_Left) && g.ActiveId == id && !hovered)
    {
        // Clicked outside - cancel
        ClearActiveID();
    }

    // Handle key capture
    if (g.ActiveId == id)
    {
        // Check keyboard keys if no mouse button was pressed
        if (!value_changed)
        {
            // Check all possible keys
            for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; i++) // only named keyboard/gamepad keys
            {
                ImGuiKey key_test = (ImGuiKey)i;

                // Skip mouse inputs
                if ((key_test >= ImGuiKey_MouseLeft && key_test <= ImGuiKey_MouseWheelY) || key_test == ImGuiKey_Escape)
                    continue;

                if (IsKeyPressed(key_test)) // Pressed, not Down, avoids "instant bind"
                {
                    *k = key_test;
                    value_changed = true;
                    ClearActiveID();
                    break;
                }
            }
        }

        // Escape cancels
        if (IsKeyPressed(ImGuiKey_Escape))
        {
            ClearActiveID();
        }
    }

    // Render

    ImGui::RenderNavHighlight(total_bb, id);

    ImVec2 buf_display_size = ImGui::CalcTextSize(buf_display, NULL, true);
    DrawLabelShadow(window->DrawList, pos + ImVec2(size.x - buf_display_size.x, size.y / 2 - buf_display_size.y / 2), it_anim->second.Label, buf_display);

    return value_changed;
}

bool ImAdd::KeyBindEx(const char* str_id, ImGuiKey* k, ImKeyBindMode* mode, const ImVec2& size_arg)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;

    bool result = ImAdd::KeyBind(str_id, k, size_arg);

    if (ImGui::IsItemActive())
    {
        if (io.MouseDown[0]) *k = ImGuiKey_MouseLeft;
        else if (io.MouseDown[1]) *k = ImGuiKey_MouseRight;
        else if (io.MouseDown[2]) *k = ImGuiKey_MouseMiddle;
        else if (io.MouseDown[3]) *k = ImGuiKey_MouseX1;
        else if (io.MouseDown[4]) *k = ImGuiKey_MouseX2;
    }

    std::string popup_name = std::string(std::string(str_id == nullptr ? "" : str_id) + std::string("##popup")).c_str();

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right, false))
    {
        ImGui::OpenPopup(popup_name.c_str());
    }
    if (ImGui::BeginPopup(popup_name.c_str()))
    {
        if (ImGui::BeginChild("PopupRect", ImVec2(ImGui::CalcTextSize("togglehold").x + style.ItemSpacing.x * 2.0f + 1.0f, ImGui::GetFontSize()), ImGuiActivateFlags_None, ImGuiWindowFlags_NoBackground))
        {
            if (ImAdd::SelectableLabel("toggle", *mode == ImKeyBindMode_Toggle)) *mode = ImKeyBindMode_Toggle;
            ImGui::SameLine(); ImAdd::VSeparator(style.WindowPadding.y, 1.0f); ImGui::SameLine();
            if (ImAdd::SelectableLabel("hold", *mode == ImKeyBindMode_Hold)) *mode = ImKeyBindMode_Hold;
        }
        ImGui::EndChild();
        ImGui::EndPopup();
    }

    return result;
}

bool ImAdd::BeginChild(const char* label, const ImVec2& size_arg)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiIO& io = g.IO;

    std::string str_label = std::string(label ? label : "");
    const ImVec2 label_size = CalcTextSize(str_label.c_str(), NULL, true);

    bool has_header = label_size.x > 0;

    bool success = ImGui::BeginChild(str_label.c_str(), size_arg, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
    if (success)
    {
        std::string body_str_id = str_label + "##body";

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = ImGui::GetWindowSize();
        ImDrawList* pDrawList = ImGui::GetWindowDrawList();

        float header_padding = style.CellPadding.x * 2.0f;
        float header_height = g.FontSize + header_padding * 2.0f;


        if (has_header)
        {
            pDrawList->AddRectFilledMultiColor(pos, pos + ImVec2(size.x, header_height), IM_COL32(40, 40, 40, 255), IM_COL32(40, 40, 40, 255), IM_COL32(24, 24, 24, 255), IM_COL32(24, 24, 24, 255));
        }

        if (has_header)
        {
            DrawLabelShadow(pDrawList, pos + ImVec2(header_padding + style.FrameBorderSize * 2.0f, header_padding + style.FrameBorderSize), GetColorU32(ImGuiCol_Text), str_label.c_str());
        }

        if (style.ChildBorderSize > 0)
        {
            pDrawList->AddRect(pos, pos + size, ImGui::GetColorU32(ImGuiCol_Border), 0, 0, 1.0f);
            pDrawList->AddRect(pos + ImVec2(1.0f, 1.0f), pos + size - ImVec2(1.0f, 1.0f), IM_COL32(44, 44, 44, 255), 0, 0, 1.0f);
            pDrawList->AddRect(pos + ImVec2(2.0f, 2.0f), pos + size - ImVec2(2.0f, 2.0f), ImGui::GetColorU32(ImGuiCol_Border), 0, 0, 1.0f);

            if (has_header) pDrawList->AddLine(pos + ImVec2(style.ChildBorderSize * 2, header_height), pos + ImVec2(size.x - style.ChildBorderSize * 2, header_height), ImGui::GetColorU32(ImGuiCol_Border), style.ChildBorderSize);
        }

        SetCursorScreenPos(pos + ImVec2(0, has_header ? header_height : 0.0f));
        PushStyleVar(ImGuiStyleVar_WindowPadding, style.ChildPadding);
        ImGui::BeginChild(body_str_id.c_str(), ImVec2(GetWindowWidth(), GetWindowHeight() - (has_header ? header_height : 0.0f)), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground);
        PushItemWidth(GetContentRegionAvail().x);
    }

    return success;
}
void ImAdd::EndChild()
{
    PopItemWidth();
    ImGui::EndChild();
    PopStyleVar();
    ImGui::EndChild();
}

void ImAdd::RenderArrow(ImDrawList* draw_list, ImVec2 pos, ImU32 col, float sz, ImGuiDir direction)
{
    if (direction < 0 || direction >= ImGuiDir_COUNT) return;

    float thickness = 1.0f;

    float half = ImTrunc(sz / 2.0f) + 0.5f;
    float pad_x = 3;
    float pad_y = 5;

    if (direction == ImGuiDir_Down)
    {
        draw_list->AddTriangleFilled(pos + ImVec2(pad_x, pad_y) - ImVec2(1, 1), pos + ImVec2(sz - pad_x, pad_y), pos + ImVec2(half, sz - pad_y) - ImVec2(1, 1), IM_COL32_BLACK);
        draw_list->AddTriangleFilled(pos + ImVec2(pad_x, pad_y) + ImVec2(1, -1), pos + ImVec2(sz - pad_x, pad_y), pos + ImVec2(half, sz - pad_y) + ImVec2(1, -1), IM_COL32_BLACK);
        draw_list->AddTriangleFilled(pos + ImVec2(pad_x, pad_y) + ImVec2(0, 1), pos + ImVec2(sz - pad_x, pad_y), pos + ImVec2(half, sz - pad_y) + ImVec2(0, 1), IM_COL32_BLACK);

        draw_list->AddTriangleFilled(pos + ImVec2(pad_x, pad_y), pos + ImVec2(sz - pad_x, pad_y), pos + ImVec2(half, sz - pad_y), col);
    }
    else if (direction == ImGuiDir_Up)
    {
        // TODO
    }
    else if (direction == ImGuiDir_Right)
    {
        // TODO
    }
    else if (direction == ImGuiDir_Left)
    {
        // TODO
    }

    draw_list->PathStroke(col, 0, thickness);
}

void ImAdd::RenderCheckMark(ImDrawList* draw_list, ImVec2 pos, ImU32 col, float sz)
{
    //float thickness = ImMax(sz / 5.0f, 1.0f);
    float thickness = 1.5f;

    sz -= thickness * 0.5f;
    pos += ImVec2(thickness * 0.25f, thickness * 0.25f);

    float third = sz / 3.0f;
    float bx = pos.x + third;
    float by = pos.y + sz - third * 0.5f;
    draw_list->PathLineTo(ImVec2(bx - third, by - third));
    draw_list->PathLineTo(ImVec2(bx, by));
    draw_list->PathLineTo(ImVec2(bx + third * 2.0f, by - third * 2.0f));
    draw_list->PathStroke(col, 0, thickness);
}

float ImAdd::GetColorPickerWidth()
{
    return (GetFontSize() + ImGui::GetStyle().CellPadding.y * 2.0f) - ImFmod(GetFontSize(), 2.0f) + 1.0f;
}

float ImAdd::CalcKeyBindWidth(const ImGuiKey& key)
{
    return CalcTextSize(std::string("[" + std::string(GetKeyName(key)) + "]").c_str()).x;
}

void ImAdd::DrawLabelShadow(ImDrawList* draw_list, ImVec2 pos, ImU32 col, const char* text)
{
    draw_list->AddText(pos + ImVec2(1, 1), IM_COL32_BLACK, text);
    draw_list->AddText(pos + ImVec2(-1, -1), IM_COL32_BLACK, text);
    draw_list->AddText(pos + ImVec2(-1, 1), IM_COL32_BLACK, text);
    draw_list->AddText(pos + ImVec2(1, -1), IM_COL32_BLACK, text);

    draw_list->AddText(pos + ImVec2(0, 1), IM_COL32_BLACK, text);
    draw_list->AddText(pos + ImVec2(1, 0), IM_COL32_BLACK, text);
    draw_list->AddText(pos + ImVec2(0, -1), IM_COL32_BLACK, text);
    draw_list->AddText(pos + ImVec2(-1, 0), IM_COL32_BLACK, text);

    draw_list->AddText(pos, col, text);
}
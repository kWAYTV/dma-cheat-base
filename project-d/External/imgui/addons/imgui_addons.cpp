#include "imgui_addons.h"

#include <map>
#include <string>
#include <windows.h>

// u gonna need update the imgui files to 1.9 wip

using namespace ImGui;

ImVec4 ImAdd::HexToColorVec4(unsigned int hex_color, float alpha) {
    ImVec4 color;

    color.x = ((hex_color >> 16) & 0xFF) / 255.0f;
    color.y = ((hex_color >> 8) & 0xFF) / 255.0f;
    color.z = (hex_color & 0xFF) / 255.0f;
    color.w = alpha;

    return color;
}

void ImAdd::DoubleText(ImVec4 color1, ImVec4 color2, const char* label1, const char* label2)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(std::string(label1 + std::string(label2)).c_str());
    const ImVec2 label1_size = CalcTextSize(label1, NULL, true);
    const ImVec2 label2_size = CalcTextSize(label2, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(ImVec2(-0.1f, g.FontSize), label1_size.x + label2_size.x, g.FontSize);

    const ImRect total_bb(pos, pos + size);
    ItemSize(total_bb);
    if (!ItemAdd(total_bb, id)) {
        return;
    }

    window->DrawList->AddText(pos, GetColorU32(color1), label1);
    window->DrawList->AddText(pos + ImVec2(size.x - ImGui::CalcTextSize(label2).x, 0), GetColorU32(color2), label2);
}

void ImAdd::TextCustomFont(ImFont* font, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    PushFont(font);
    TextV(fmt, args);
    PopFont();
    va_end(args);
}

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

void ImAdd::VSeparator(float thickness)
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
    const ImRect bb_rect(pos + ImVec2(0, style.FramePadding.y), pos + size - ImVec2(0, style.FramePadding.y));

    ItemSize(ImVec2(thickness, 0.0f));
    if (!ItemAdd(bb, 0))
        return;

    window->DrawList->AddRectFilled(bb_rect.Min, bb_rect.Max, GetColorU32(ImGuiCol_Border));
}

bool ImAdd::RadioFrameIcon(const char* label, ImTextureID user_texture_id, int* v, int current_id, const ImVec2& size_arg, const ImVec2& adjust_icon_pos)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImVec2 icon_size = ImVec2(g.FontSize, g.FontSize);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + icon_size.x + style.FramePadding.x * 3.0f, g.FontSize + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held, active = *v == current_id;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    if (pressed)
        *v = current_id;

    // Colors
    ImVec4 colFrameMain = GetStyleColorVec4(ImGuiCol_FrameBg);
    ImVec4 colFrameNull = colFrameMain; colFrameNull.w = 0.0f;
    ImVec4 colFrame = (active ? colFrameMain : colFrameNull);

    ImVec4 colBorderMain = GetStyleColorVec4(ImGuiCol_Border);
    ImVec4 colBorderNull = colBorderMain; colBorderNull.w = 0.0f;
    ImVec4 colBorder = (active ? colBorderMain : colBorderNull);

    ImVec4 colIcon = GetStyleColorVec4(active ? ImGuiCol_SliderGrab : (hovered || held) ? ImGuiCol_Text : ImGuiCol_TextDisabled);
    ImVec4 colLabel = GetStyleColorVec4((hovered || held || active) ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    // Animations
    struct stColors_State { 
        ImVec4 Frame;
        ImVec4 Border;
        ImVec4 Label;
        ImVec4 Icon;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
        it_anim->second.Border = colBorder;
        it_anim->second.Label = colLabel;
        it_anim->second.Icon = colIcon;
    }

    it_anim->second.Frame = ImLerp(it_anim->second.Frame, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.Border = ImLerp(it_anim->second.Border, colBorder, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.Label = ImLerp(it_anim->second.Label, colLabel, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.Icon = ImLerp(it_anim->second.Icon, colIcon, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);

    // Render
    RenderNavHighlight(bb, id);

    window->DrawList->AddRectFilled(pos, pos + size, GetColorU32(it_anim->second.Frame), style.FrameRounding);
    if (style.FrameBorderSize)
        window->DrawList->AddRect(pos, pos + size, GetColorU32(it_anim->second.Border), style.FrameRounding, 0, style.FrameBorderSize);

    if (label_size.x > 0)
    {
        window->DrawList->AddImage(user_texture_id, pos + style.FramePadding + adjust_icon_pos, pos + style.FramePadding + icon_size + adjust_icon_pos, ImVec2(0, 0), ImVec2(1, 1), GetColorU32(it_anim->second.Icon));
        window->DrawList->AddText(pos + ImVec2(size.y, style.FramePadding.y), GetColorU32(it_anim->second.Label), label);
    }
    else
    {
        window->DrawList->AddImage(user_texture_id, pos + style.FramePadding + adjust_icon_pos, pos + style.FramePadding + icon_size + adjust_icon_pos, ImVec2(0, 0), ImVec2(1, 1), GetColorU32(it_anim->second.Icon));
    }

    return pressed;
}

bool ImAdd::RadioFrame(const char* label, int* v, int current_id, bool borders, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, g.FontSize + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held, active = *v == current_id;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    if (pressed)
        *v = current_id;

    // Colors
    /*
    ImVec4 colFrameMain = GetStyleColorVec4(ImGuiCol_FrameBg);
    ImVec4 colFrameNull = colFrameMain; colFrameNull.w = 0.0f;
    ImVec4 colFrame = (active ? colFrameMain : colFrameNull);
    */
    ImVec4 colFrame = GetStyleColorVec4(active ? ImGuiCol_SliderGrab : (held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_ChildBg);

    /*
    ImVec4 colBorderMain = GetStyleColorVec4(ImGuiCol_Border);
    ImVec4 colBorderNull = colBorderMain; colBorderNull.w = 0.0f;
    ImVec4 colBorder = (active ? colBorderMain : colBorderNull);
    */

    ImVec4 colLabel = GetStyleColorVec4((hovered || held || active) ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    // Animations
    struct stColors_State {
        ImVec4 Frame;
        //ImVec4 Border;
        ImVec4 Label;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
        //it_anim->second.Border = colBorder;
        it_anim->second.Label = colLabel;
    }

    it_anim->second.Frame = ImLerp(it_anim->second.Frame, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    //it_anim->second.Border = ImLerp(it_anim->second.Border, colBorder, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.Label = ImLerp(it_anim->second.Label, colLabel, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);

    // Render
    RenderNavHighlight(bb, id);

    window->DrawList->AddRectFilled(pos, pos + size, GetColorU32(it_anim->second.Frame), style.FrameRounding);
    if (style.FrameBorderSize && borders)
        window->DrawList->AddRect(pos, pos + size, GetColorU32(ImGuiCol_Border), style.FrameRounding, 0, style.FrameBorderSize);

    if (label_size.x > 0)
    {
        window->DrawList->AddText(pos + ImVec2(size.x / 2 - label_size.x / 2, style.FramePadding.y), GetColorU32(it_anim->second.Label), label);
    }

    return pressed;
}

bool ImAdd::RadioFrameColor(const char* str_id, int* v, int current_id, ImVec4 accent_color, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(str_id);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, GetFrameHeight(), GetFrameHeight());

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    bool active = *v == current_id;

    if (pressed)
        *v = current_id;

    // Render
    RenderNavHighlight(bb, id);

    ImVec2 border_padding = ImVec2(0, 0);
    if (active)
        border_padding = ImVec2(2, 2);

    window->DrawList->AddRectFilled(pos + border_padding, pos + size - border_padding, GetColorU32(accent_color), style.FrameRounding);

    if (active)
        window->DrawList->AddRect(pos, pos + size, GetColorU32(ImGuiCol_SliderGrab), style.FrameRounding, 0, 1);

    return pressed;
}

bool ImAdd::InputText(const char* label, const char* text, char* buf, size_t buf_size, float width, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImVec2 text_size = CalcTextSize(text, NULL, true);

    IM_ASSERT(!(flags & ImGuiInputTextFlags_Multiline)); // call InputTextMultiline()

    bool result = false;
    bool has_label = label_size.x > 0;

    const float w = CalcItemSize(ImVec2(width, 0), CalcItemWidth(), 0).x;

    BeginGroup();
    {
        if (has_label) {
            PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.ItemInnerSpacing.y));
            ImGui::Text(label);
        }
        {
            ImVec2 pos = GetCursorScreenPos();
            PushItemWidth(w);
            {
                result |= InputTextEx(std::string("##" + std::string(label)).c_str(), NULL, buf, (int)buf_size, ImVec2(0, 0), flags, callback, user_data);
            }
            PopItemWidth();

            if (text_size.x > 0)
            {
                if (!ImGui::IsItemActive() && !strlen(buf)) {
                    ImGui::SetCursorScreenPos(pos + style.FramePadding);
                    ImGui::TextDisabled(text);
                }
            }
        }
        if (has_label) {
            PopStyleVar();
        }
    }
    EndGroup();

    return result;
}

bool ImAdd::Button(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Colors
    ImVec4 colFrame = GetStyleColorVec4((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    // Animations
    struct stColors_State {
        ImVec4 Frame;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
    }

    it_anim->second.Frame = ImLerp(it_anim->second.Frame, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);

    // Render
    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, GetColorU32(it_anim->second.Frame), true, style.FrameRounding);

    if (g.LogEnabled)
        LogSetNextTextDecoration("[", "]");
    RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    return pressed;
}

bool ImAdd::ColorEdit4(const char* label, float col[4])
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImVec4 col_v4(col[0], col[1], col[2], col[3]);

    ImVec2 size = CalcItemSize(ImVec2(-0.1f, g.FontSize), label_size.x + style.ItemInnerSpacing.x + g.FontSize * 2, g.FontSize);

    ImGuiColorEditFlags flags = ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview;

    BeginGroup();

    if (label_size.x > 0) {
        Text(label);
        SameLine(size.x - g.FontSize * 2);
    }

    bool result = ColorButton(label, col_v4, flags, ImVec2(g.FontSize * 2, g.FontSize));
    if (result)
    {
        OpenPopup(std::string(std::string(label) + "##picker").c_str());
    }
    if (BeginPopup(std::string(std::string(label) + "##picker").c_str()))
    {
        ColorPicker4(label, col, flags);
        EndPopup();
    }

    EndGroup();

    return result;
}

bool ImAdd::ToggleButtonClassic(const char* label, bool* v)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    float height = g.FontSize;
    float width = height * 2;
    float rounding = height / 2;

    float grab_radius = (height / 2) - 3;
    float grab_rounding = grab_radius / 2;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImVec2(label_size.x > 0 ? label_size.x + style.ItemInnerSpacing.x + width : width, height);

    const ImRect bb(pos, pos + size);
    ItemSize(size);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    if (pressed)
        *v = !*v;

    // Colors
    ImVec4 colFrame = GetStyleColorVec4(*v ? ImGuiCol_SliderGrab : ImGuiCol_FrameBg);
    ImVec4 colBorder = GetStyleColorVec4(*v ? ImGuiCol_SliderGrabActive : ImGuiCol_Border);
    ImVec4 colGrab = GetStyleColorVec4(*v ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    // Animations
    struct stColors_State {
        ImVec4 Frame;
        ImVec4 Border;
        ImVec4 Grab;
        float GrabPos;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
        it_anim->second.Border = colBorder;
        it_anim->second.Grab = colGrab;
        it_anim->second.GrabPos = *v ? 1.0f : 0.0f;
    }

    it_anim->second.Frame = ImLerp(it_anim->second.Frame, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.Border = ImLerp(it_anim->second.Border, colBorder, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.Grab = ImLerp(it_anim->second.Grab, colGrab, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.GrabPos = ImLerp<float>(it_anim->second.GrabPos, *v ? (width - rounding) : rounding, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);

    // Render
    RenderNavHighlight(bb, id);

    window->DrawList->AddRectFilled(pos, pos + ImVec2(width, size.y), GetColorU32(it_anim->second.Frame), rounding);
    if (style.FrameBorderSize)
        window->DrawList->AddRect(pos, pos + ImVec2(width, size.y), GetColorU32(it_anim->second.Border), rounding, 0, style.FrameBorderSize);

    float grab_pos = *v ? (width - rounding * 2) : 0;

    window->DrawList->AddCircleFilled(pos + ImVec2(it_anim->second.GrabPos, rounding), grab_radius, GetColorU32(it_anim->second.Grab));

    if (label_size.x > 0)
    {
        RenderText(pos + ImVec2(width + style.ItemInnerSpacing.x, 0), label);
    }

    return pressed;
}

bool ImAdd::ToggleButton(const char* label, bool* v, float spacing_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    float height = g.FontSize;
    float width = height * 2;
    float rounding = height / 2;

    float spacing = ImGui::CalcItemSize(ImVec2(spacing_arg > 0 ? spacing_arg : -0.1f, 0), label_size.x > 0 ? (label_size.x + style.ItemInnerSpacing.x) : 0, 0).x;

    float grab_radius = (height / 2) - 3;
    float grab_rounding = grab_radius / 2;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImVec2(label_size.x > 0 ? label_size.x + style.ItemInnerSpacing.x + width : width, height);

    const ImRect bb(pos, pos + ImVec2(spacing, size.y));
    ItemSize(size);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    if (pressed)
        *v = !*v;

    // Colors
    ImVec4 colFrame = GetStyleColorVec4(*v ? ImGuiCol_SliderGrab : ImGuiCol_FrameBg);
    ImVec4 colBorder = GetStyleColorVec4(*v ? ImGuiCol_SliderGrabActive : ImGuiCol_Border);
    ImVec4 colGrab = GetStyleColorVec4(*v ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    // Animations
    struct stColors_State {
        ImVec4 Frame;
        ImVec4 Border;
        ImVec4 Grab;
        float GrabPos;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
        it_anim->second.Border = colBorder;
        it_anim->second.Grab = colGrab;
        it_anim->second.GrabPos = *v ? 1.0f : 0.0f;
    }

    it_anim->second.Frame = ImLerp(it_anim->second.Frame, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.Border = ImLerp(it_anim->second.Border, colBorder, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.Grab = ImLerp(it_anim->second.Grab, colGrab, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.GrabPos = ImLerp<float>(it_anim->second.GrabPos, *v ? (width - rounding) : rounding, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);

    // Render
    RenderNavHighlight(bb, id);

    window->DrawList->AddRectFilled(pos + ImVec2(spacing - width, 0), pos + ImVec2(spacing - width, 0) + ImVec2(width, size.y), GetColorU32(it_anim->second.Frame), rounding);
    if (style.FrameBorderSize)
        window->DrawList->AddRect(pos + ImVec2(spacing - width, 0), pos + ImVec2(spacing - width, 0) + ImVec2(width, size.y), GetColorU32(it_anim->second.Border), rounding, 0, style.FrameBorderSize);

    float grab_pos = *v ? (width - rounding * 2) : 0;

    window->DrawList->AddCircleFilled(pos + ImVec2(spacing - width, 0) + ImVec2(it_anim->second.GrabPos, rounding), grab_radius, GetColorU32(it_anim->second.Grab));

    if (label_size.x > 0)
    {
        RenderText(pos, label);
    }

    return pressed;
}

bool ImAdd::CheckBox(const char* label, bool* v)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    float height = g.FontSize;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImVec2(label_size.x > 0 ? label_size.x + style.ItemInnerSpacing.x + height : height, height);

    const ImRect bb(pos, pos + size);
    ItemSize(size);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    if (pressed)
        *v = !*v;

    // Colors
    ImVec4 colFrame = GetStyleColorVec4(*v ? ImGuiCol_SliderGrabActive : (hovered && !held) ? ImGuiCol_FrameBgHovered : held ? ImGuiCol_FrameBgActive : ImGuiCol_FrameBg);
    ImVec4 colBorder = GetStyleColorVec4(*v ? ImGuiCol_SliderGrab : ImGuiCol_Border);
    ImVec4 colCheckMarkMain = GetStyleColorVec4(ImGuiCol_CheckMark);
    ImVec4 colCheckMarkNull = colCheckMarkMain; colCheckMarkNull.w = 0.0f;
    ImVec4 colCheckMark = (*v ? colCheckMarkMain : colCheckMarkNull);

    // Animations
    struct stColors_State {
        ImVec4 Frame;
        ImVec4 Border;
        ImVec4 CheckMark;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
        it_anim->second.Border = colBorder;
        it_anim->second.CheckMark = colCheckMark;
    }

    it_anim->second.Frame = ImLerp(it_anim->second.Frame, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.Border = ImLerp(it_anim->second.Border, colBorder, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.CheckMark = ImLerp(it_anim->second.CheckMark, colCheckMark, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);

    // Render
    RenderNavHighlight(bb, id);

    window->DrawList->AddRectFilled(pos, pos + ImVec2(height, size.y), GetColorU32(it_anim->second.Frame), style.FrameRounding);
    if (style.FrameBorderSize)
        window->DrawList->AddRect(pos, pos + ImVec2(height, size.y), GetColorU32(it_anim->second.Border), style.FrameRounding, 0, style.FrameBorderSize);

    float pad = ImMax(1.0f, IM_TRUNC(height / 4.0f));
    RenderCheckMark(window->DrawList, pos + ImVec2(pad, pad), GetColorU32(it_anim->second.CheckMark), height - pad * 2.0f);

    if (label_size.x > 0)
    {
        RenderText(pos + ImVec2(height + style.ItemInnerSpacing.x, 0), label);
    }

    return pressed;
}

bool ImAdd::SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, float width, const char* format)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    
    char value_buf[64];
    const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    float w = CalcItemSize(ImVec2(width, 0), CalcItemWidth(), 0).x;
    w -= label_size.x > 0 ? 0 : CalcTextSize(value_buf).x + style.ItemInnerSpacing.x;

    ImVec2 pos = window->DC.CursorPos;

    const float label_height = label_size.x > 0 ? g.FontSize + style.ItemInnerSpacing.y : 0.0f;
    const ImRect frame_bb(pos + ImVec2(0, label_height), pos + ImVec2(w, g.FontSize + label_height));
    const ImRect total_bb(pos, pos + ImVec2(w, g.FontSize + label_height));

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &frame_bb))
        return false;

    // Default format string when passing NULL
    if (format == NULL)
        format = DataTypeGetInfo(data_type)->PrintFmt;

    const bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.InFlags);
    const bool clicked = hovered && IsMouseClicked(0, id);
    const bool make_active = (clicked || g.NavActivateId == id);

    if (make_active)
    {
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    // Colors
    ImVec4 colFrame = GetStyleColorVec4(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    ImVec4 colGrab = GetStyleColorVec4(g.ActiveId == id ? ImGuiCol_SliderGrab : ImGuiCol_SliderGrabActive);

    // Animations
    struct stColors_State {
        ImVec4 Frame;
        ImVec4 Grab;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
        it_anim->second.Grab = colGrab;
    }

    it_anim->second.Frame = ImLerp(it_anim->second.Frame, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.Grab = ImLerp(it_anim->second.Grab, colGrab, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);

    // Render
    RenderNavHighlight(frame_bb, id);
    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(it_anim->second.Frame), true, g.Style.FrameRounding);

    // Slider behavior
    ImRect grab_bb;
    const bool value_changed = SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, 0, &grab_bb);
    if (value_changed)
        MarkItemEdited(id);

    ImVec2 grab_padding = ImVec2(grab_bb.Min.y - frame_bb.Min.y, 0); grab_padding.y = grab_padding.x;

    // Render grab
    //if (grab_bb.Max.x > grab_bb.Min.x)
    {
        window->DrawList->AddRectFilled(frame_bb.Min, grab_bb.Max + grab_padding, GetColorU32(it_anim->second.Grab), style.FrameRounding);

        if (style.FrameBorderSize > 0)
        {
            window->DrawList->AddRect(frame_bb.Min, grab_bb.Max + grab_padding, GetColorU32(ImGuiCol_SliderGrab), style.FrameRounding, 0, style.FrameBorderSize);
        }
    }

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    float extra_width;
    if (label_size.x > 0) {
        extra_width = -ImGui::CalcTextSize(value_buf).x;
    }
    else {
        extra_width = style.ItemInnerSpacing.x;
    }

    window->DrawList->AddText(pos + ImVec2(w + extra_width, 0), GetColorU32(ImGuiCol_TextDisabled), value_buf);

    if (label_size.x > 0.0f)
        RenderText(pos, label);

    return value_changed;
}

bool ImAdd::SliderFloat(const char* label, float* v, float v_min, float v_max, float width, const char* format)
{
    return SliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, width, format);
}

bool ImAdd::SliderInt(const char* label, int* v, int v_min, int v_max, float width, const char* format)
{
    return SliderScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, width, format);
}

bool ImAdd::Selectable(const char* label, bool selected, const ImVec2& size_arg)
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

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    float borderSize = style.FrameBorderSize;

    // Colors
    ImVec4 colFrameMain = GetStyleColorVec4((hovered && !selected) ? held ? ImGuiCol_HeaderActive : ImGuiCol_HeaderHovered : ImGuiCol_Header);
    ImVec4 colFrameNull = colFrameMain; colFrameNull.w = 0.0f;
    ImVec4 colFrame = ((!hovered && !selected) ? colFrameNull : colFrameMain);

    ImVec4 colBorderMain = GetStyleColorVec4(ImGuiCol_Border);
    ImVec4 colBorderNull = colBorderMain; colBorderNull.w = 0.0f;
    ImVec4 colBorder = (selected ? colBorderMain : colBorderNull);

    // Animations
    struct stColors_State {
        ImVec4 Frame;
        ImVec4 Border;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
        it_anim->second.Border = colBorder;
    }

    it_anim->second.Frame = ImLerp(it_anim->second.Frame, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.Border = ImLerp(it_anim->second.Border, colBorder, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);

    // Render
    RenderNavHighlight(bb, id);

    window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(it_anim->second.Frame), style.FrameRounding);

    if (borderSize > 0)
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(it_anim->second.Border), style.FrameRounding, 0, borderSize);

    RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    return pressed;
}

bool ImAdd::BeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();

    ImGuiNextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.Flags;
    g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together
    if (flags & ImGuiComboFlags_WidthFitPreview)
        IM_ASSERT((flags & (ImGuiComboFlags_NoPreview | ImGuiComboFlags_CustomPreview)) == 0);

    const float arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float preview_width = ((flags & ImGuiComboFlags_WidthFitPreview) && (preview_value != NULL)) ? CalcTextSize(preview_value, NULL, true).x : 0.0f;
    const float w = (flags & ImGuiComboFlags_NoPreview) ? arrow_size : ((flags & ImGuiComboFlags_WidthFitPreview) ? (arrow_size + preview_width + style.FramePadding.x * 2.0f) : CalcItemWidth());

    const ImRect bb(window->DC.CursorPos + ImVec2(0.0f, label_size.x > 0 ? label_size.y + style.ItemInnerSpacing.y : 0.0f), window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f) + ImVec2(0.0f, label_size.x > 0 ? label_size.y + style.ItemInnerSpacing.y : 0.0f));
    const ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f) + ImVec2(0.0f, label_size.x > 0 ? label_size.y + style.ItemInnerSpacing.y : 0.0f));

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &bb))
        return false;

    // Open on click
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id);
    bool popup_open = IsPopupOpen(popup_id, ImGuiPopupFlags_None);
    if (pressed && !popup_open)
    {
        OpenPopupEx(popup_id, ImGuiPopupFlags_None);
        popup_open = true;
    }

    // Colors
    ImVec4 colFrame = GetStyleColorVec4(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    ImVec4 colText = GetStyleColorVec4((popup_open || hovered) ? ImGuiCol_Text : ImGuiCol_TextDisabled);

    // Animations
    struct stColors_State {
        ImVec4 Frame;
        ImVec4 Text;
    };

    static std::map<ImGuiID, stColors_State> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, stColors_State() });
        it_anim = anim.find(id);

        it_anim->second.Frame = colFrame;
        it_anim->second.Text = colText;
    }

    it_anim->second.Frame = ImLerp(it_anim->second.Frame, colFrame, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);
    it_anim->second.Text = ImLerp(it_anim->second.Text, colText, 1.0f / IMADD_ANIMATIONS_SPEED * ImGui::GetIO().DeltaTime);

    // Render shape
    const float value_x2 = ImMax(bb.Min.x, bb.Max.x - arrow_size);
    RenderNavHighlight(bb, id);
    if (!(flags & ImGuiComboFlags_NoPreview))
        window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(it_anim->second.Frame), style.FrameRounding);
    if (!(flags & ImGuiComboFlags_NoArrowButton))
    {
        if (value_x2 + arrow_size - style.FramePadding.x <= bb.Max.x)
            RenderArrow(window->DrawList, ImVec2(value_x2 + style.FramePadding.y, bb.Min.y + style.FramePadding.y), GetColorU32(it_anim->second.Text), ImGuiDir_Down, 1.0f);
    }
    RenderFrameBorder(bb.Min, bb.Max, style.FrameRounding);

    // Custom preview
    if (flags & ImGuiComboFlags_CustomPreview)
    {
        g.ComboPreviewData.PreviewRect = ImRect(bb.Min.x, bb.Min.y, value_x2, bb.Max.y);
        IM_ASSERT(preview_value == NULL || preview_value[0] == 0);
        preview_value = NULL;
    }

    // Render preview and label
    if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
    {
        if (g.LogEnabled)
            LogSetNextTextDecoration("{", "}");
        PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled));
        RenderTextClipped(bb.Min + style.FramePadding, ImVec2(value_x2, bb.Max.y), preview_value, NULL, NULL);
        PopStyleColor();
    }
    if (label_size.x > 0)
        RenderText(total_bb.Min, label);

    if (!popup_open)
        return false;

    g.NextWindowData.Flags = backup_next_window_data_flags;
    return BeginComboPopup(popup_id, bb, flags);
}

static float CalcMaxPopupHeightFromItemCount(int items_count)
{
    ImGuiContext& g = *GImGui;
    if (items_count <= 0)
        return FLT_MAX;
    return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

bool ImAdd::BeginComboPopup(ImGuiID popup_id, const ImRect& bb, ImGuiComboFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (!IsPopupOpen(popup_id, ImGuiPopupFlags_None))
    {
        g.NextWindowData.ClearFlags();
        return false;
    }

    // Set popup size
    float w = bb.GetWidth();
    if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint)
    {
        g.NextWindowData.SizeConstraintRect.Min.x = ImMax(g.NextWindowData.SizeConstraintRect.Min.x, w);
    }
    else
    {
        if ((flags & ImGuiComboFlags_HeightMask_) == 0)
            flags |= ImGuiComboFlags_HeightRegular;
        IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_)); // Only one
        int popup_max_height_in_items = -1;
        if (flags & ImGuiComboFlags_HeightRegular)     popup_max_height_in_items = 8;
        else if (flags & ImGuiComboFlags_HeightSmall)  popup_max_height_in_items = 4;
        else if (flags & ImGuiComboFlags_HeightLarge)  popup_max_height_in_items = 20;
        ImVec2 constraint_min(0.0f, 0.0f), constraint_max(FLT_MAX, FLT_MAX);
        if ((g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSize) == 0 || g.NextWindowData.SizeVal.x <= 0.0f) // Don't apply constraints if user specified a size
            constraint_min.x = w;
        if ((g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSize) == 0 || g.NextWindowData.SizeVal.y <= 0.0f)
            constraint_max.y = CalcMaxPopupHeightFromItemCount(popup_max_height_in_items);
        SetNextWindowSizeConstraints(constraint_min, constraint_max);
    }

    // This is essentially a specialized version of BeginPopupEx()
    char name[16];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // Recycle windows based on depth

    // Set position given a custom constraint (peak into expected window size so we can position it)
    // FIXME: This might be easier to express with an hypothetical SetNextWindowPosConstraints() function?
    // FIXME: This might be moved to Begin() or at least around the same spot where Tooltips and other Popups are calling FindBestWindowPosForPopupEx()?
    if (ImGuiWindow* popup_window = FindWindowByName(name))
        if (popup_window->WasActive)
        {
            // Always override 'AutoPosLastDirection' to not leave a chance for a past value to affect us.
            ImVec2 size_expected = CalcWindowNextAutoFitSize(popup_window);
            popup_window->AutoPosLastDirection = (flags & ImGuiComboFlags_PopupAlignLeft) ? ImGuiDir_Left : ImGuiDir_Down; // Left = "Below, Toward Left", Down = "Below, Toward Right (default)"
            ImRect r_outer = GetPopupAllowedExtentRect(popup_window);
            ImVec2 pos = FindBestWindowPosForPopupEx(bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, bb, ImGuiPopupPositionPolicy_ComboBox);
            SetNextWindowPos(pos + ImVec2(0, g.Style.ItemSpacing.y));
        }

    // We don't use BeginPopupEx() solely because we have a custom name string, which we could make an argument to BeginPopupEx()
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
    PushStyleVar(ImGuiStyleVar_WindowPadding, g.Style.FramePadding);
    PushStyleVar(ImGuiStyleVar_PopupRounding, g.Style.FrameRounding);
    bool ret = Begin(name, NULL, window_flags);
    PopStyleVar(2);
    if (!ret)
    {
        EndPopup();
        IM_ASSERT(0);   // This should never happen as we tested for IsPopupOpen() above
        return false;
    }
    return true;
}

bool ImAdd::Combo(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int popup_max_height_in_items)
{
    ImGuiContext& g = *GImGui;

    // Call the getter to obtain the preview string which is a parameter to BeginCombo()
    const char* preview_value = NULL;
    if (*current_item >= 0 && *current_item < items_count)
        preview_value = getter(user_data, *current_item);

    // The old Combo() API exposed "popup_max_height_in_items". The new more general BeginCombo() API doesn't have/need it, but we emulate it here.
    if (popup_max_height_in_items != -1 && !(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
        SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));

    if (!BeginCombo(label, preview_value, ImGuiComboFlags_None))
        return false;

    // Display items
    // FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is processed)
    bool value_changed = false;
    for (int i = 0; i < items_count; i++)
    {
        const char* item_text = getter(user_data, i);
        if (item_text == NULL)
            item_text = "*Unknown item*";

        PushID(i);
        const bool item_selected = (i == *current_item);
        if (Selectable(item_text, item_selected, ImVec2(-0.1f, 0)) && *current_item != i)
        {
            value_changed = true;
            *current_item = i;
            //CloseCurrentPopup();
        }
        if (item_selected)
            SetItemDefaultFocus();
        PopID();
    }

    EndCombo();

    if (value_changed)
        MarkItemEdited(g.LastItemData.ID);

    return value_changed;
}

// Getter for the old Combo() API: const char*[]
static const char* Items_ArrayGetter(void* data, int idx)
{
    const char* const* items = (const char* const*)data;
    return items[idx];
}

// Getter for the old Combo() API: "item1\0item2\0item3\0"
static const char* Items_SingleStringGetter(void* data, int idx)
{
    const char* items_separated_by_zeros = (const char*)data;
    int items_count = 0;
    const char* p = items_separated_by_zeros;
    while (*p)
    {
        if (idx == items_count)
            break;
        p += strlen(p) + 1;
        items_count++;
    }
    return *p ? p : NULL;
}

bool ImAdd::Combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items)
{
    const bool value_changed = Combo(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_in_items);
    return value_changed;
}

bool ImAdd::Combo(const char* label, int* current_item, const char* items_separated_by_zeros, float width, int height_in_items)
{
    int items_count = 0;
    const char* p = items_separated_by_zeros;       // FIXME-OPT: Avoid computing this, or at least only when combo is open
    while (*p)
    {
        p += strlen(p) + 1;
        items_count++;
    }

    const float w = CalcItemSize(ImVec2(width, 0), CalcItemWidth(), 0).x;
    PushItemWidth(w);
    bool value_changed = Combo(label, current_item, Items_SingleStringGetter, (void*)items_separated_by_zeros, items_count, height_in_items);
    PopItemWidth();

    return value_changed;
}

bool ImAdd::KeyBind(const char* label, int* k, float custom_width, KeyBindOptions* options)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    char buf_display[32] = "None";
    ImVec2 pos = window->DC.CursorPos;
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImVec2 buf_display_size = ImGui::CalcTextSize(buf_display, NULL, true);
    float width = custom_width == 0 ? ImGui::CalcItemSize(ImVec2(-0.1, 0), 0, 0).x : custom_width;
    float height = ImGui::GetFrameHeight();

    ImVec2 size = ImVec2(width, height);
    ImRect frame_bb(pos + ImVec2(0, label_size.y + style.ItemInnerSpacing.y), pos + size + ImVec2(0, label_size.y + style.ItemInnerSpacing.y));
    ImRect total_bb(pos, frame_bb.Max + ImVec2(size.x, 0));

    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    const bool hovered = ImGui::ItemHoverable(frame_bb, id, 0);

    if (hovered)
    {
        ImGui::SetHoveredID(id);
        g.MouseCursor = ImGuiMouseCursor_Hand;
    }

    const bool user_clicked = hovered && io.MouseClicked[0];

    if (user_clicked)
    {
        if (g.ActiveId != id)
        {
            memset(io.MouseDown, 0, sizeof(io.MouseDown));
            memset(io.KeysDown, 0, sizeof(io.KeysDown));
            *k = 0x0;
        }
        ImGui::SetActiveID(id, window);
        ImGui::FocusWindow(window);
    }
    else if (io.MouseClicked[0])
    {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
    }

    bool value_changed = false;
    int key = *k;

    if (g.ActiveId == id)
    {
        for (auto i = 0; i < 5; i++)
        {
            if (io.MouseDown[i])
            {
                switch (i) {
                case 0:
                    key = 0x01;
                    break;
                case 1:
                    key = 0x02;
                    break;
                case 2:
                    key = 0x04;
                    break;
                case 3:
                    key = 0x05;
                    break;
                case 4:
                    key = 0x06;
                    break;
                }
                value_changed = true;
                ImGui::ClearActiveID();
            }
        }

        if (!value_changed)
        {
            for (auto i = 0x08; i <= 0xA5; i++)
            {
                if (io.KeysDown[i])
                {
                    key = i;
                    value_changed = true;
                    ImGui::ClearActiveID();
                }
            }
        }

        if (ImGui::IsKeyPressedMap(ImGuiKey_Escape))
        {
            *k = 0;
            ImGui::ClearActiveID();
        }
        else *k = key;
    }

    window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_Button), style.FrameRounding);

    const float border_size = g.Style.FrameBorderSize;
    if (border_size > 0.0f)
    {
        window->DrawList->AddRect(frame_bb.Min + ImVec2(1, 1), frame_bb.Max + ImVec2(1, 1), ImGui::GetColorU32(ImGuiCol_BorderShadow), style.FrameBorderSize, 0, border_size);
        window->DrawList->AddRect(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_Border), style.FrameBorderSize, 0, border_size);
    }

    ImGui::RenderNavHighlight(total_bb, id);

    if (*k != 0 && g.ActiveId != id)
        strcpy_s(buf_display, szKeyNames[*k]);
    else if (g.ActiveId == id)
        strcpy_s(buf_display, "Press any key");

    const ImRect clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + size.x, frame_bb.Min.y + size.y);
    ImGui::RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, NULL, NULL, style.ButtonTextAlign, &clip_rect);

    ImVec2 label_pos = pos;
    if (label_size.x > 0.0f)
        ImGui::RenderText(label_pos, label);

    if (hovered && ImGui::IsMouseReleased(1))
    {
        ImGui::OpenPopup("Mode");
    }

    if (ImGui::BeginPopup("Mode"))
    {
        if (ImGui::MenuItem("Always On", nullptr, *options == KeyBindOptions::Always))
        {
            *options = KeyBindOptions::Always;
        }
        if (ImGui::MenuItem("On Toggle", nullptr, *options == KeyBindOptions::OnToggle))
        {
            *options = KeyBindOptions::OnToggle;
        }
        if (ImGui::MenuItem("On Key", nullptr, *options == KeyBindOptions::OnKeyDown))
        {
            *options = KeyBindOptions::OnKeyDown;
        }
        ImGui::EndPopup();
    }

    return value_changed;
}

bool ImAdd::MultiComboBox(const char* label, const char* items[], std::vector<bool>& v, int size)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* pWindow = g.CurrentWindow;
    if (pWindow->SkipItems)
        return false;

    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;

    std::string szBuffer;
    const ImVec2 vecLabelSize = ImGui::CalcTextSize(label);
    float flActiveWidth = ImGui::CalcItemWidth() - (style.ItemInnerSpacing.x + ImGui::GetFrameHeight()) - 10.f;

    for (int i = 0; i < size; i++)
    {
        if (v[i])
        {
            ImVec2 vecTextSize = ImGui::CalcTextSize(szBuffer.c_str());

            if (szBuffer.empty())
                szBuffer.assign(items[i]);
            else
                szBuffer.append(", ").append(items[i]);

            if (vecTextSize.x > flActiveWidth)
                szBuffer.erase(szBuffer.find_last_of(",")).append("...");
        }
    }

    if (szBuffer.empty())
        szBuffer.assign("None");

    bool bValueChanged = false;
    if (ImGui::BeginCombo(label, szBuffer.c_str()))
    {
        for (int i = 0; i < size; i++)
        {
            if (ImGui::Selectable(items[i], v[i], ImGuiSelectableFlags_DontClosePopups))
            {
                v[i] = !v[i];
                bValueChanged = true;
            }
        }

        ImGui::EndCombo();
    }

    return bValueChanged;
}

void ImAdd::Hint(const char* desc)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

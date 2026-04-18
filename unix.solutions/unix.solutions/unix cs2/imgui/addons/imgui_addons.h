//============ Copyright KiwiHax, All rights reserved ============//
//
//  Purpose: 
//
//================================================================//

#pragma once

#include "../imgui.h"

#define IMADD_ANIMATIONS_SPEED	0.07f

#include <vector>

enum ImKeyBindMode : int
{
	ImKeyBindMode_None = 0, // Default = Toggle

	ImKeyBindMode_Toggle = ImKeyBindMode_None,
	ImKeyBindMode_Hold
};

namespace ImAdd
{
	// Separators
	void    SeparatorText(const char* label, float thickness = 1.0f);
	void    VSeparator(float margin = 0.0f, float thickness = 1.0f);

	// Selectables
	bool	SelectableLabel(const char* label, bool selected = false, const ImVec2& size_arg = ImVec2(0, 0));
	bool	SelectableFrame(const char* label, bool selected = false, const ImVec2& size_arg = ImVec2(0, 0));

	// Togglables
	bool	CheckBox(const char* label, bool* checked);

	// Buttons
	bool	Button(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImDrawFlags draw_flags = 0, ImGuiButtonFlags button_flags = 0);

	// Sliders
	bool	SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL);
	bool	SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.1f");
	bool	SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d");

	// Drags
	bool	DragScalar(const char* label, ImGuiDataType data_type, void* p_data, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL);
	bool	DragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.1f"); 
	bool	DragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d");

	// Inputs
	bool	InputText(const char* label, const char* placeholder, char* buf, size_t buf_size, ImDrawFlags draw_flags = 0, ImGuiInputTextFlags input_flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);

	// Combos
	bool	Combo(const char* label, int* selected_index, std::vector<const char*> items);

	// Color Stuff
	bool	SliderAlpha(const char* str_id, ImVec4& col);
	bool	SliderHue(const char* str_id, ImVec4& col);
	bool	ColorPalette(const char* str_id, ImVec4& col, const ImVec2& size_arg = ImVec2(0, 0));
	bool	ColorButton(const char* desc_id, const ImVec4& col, bool has_alpha, const ImVec2& size_arg = ImVec2(0, 0));
	void	ColorPicker4(const char* label, float col[4], bool has_alpha);
	bool    ColorEdit4(const char* name, float col[4]);

	bool	KeyBind(const char* str_id, ImGuiKey* k, const ImVec2& size_arg = ImVec2(0, 0));
	bool	KeyBindEx(const char* str_id, ImGuiKey* k, ImKeyBindMode* mode, const ImVec2& size_arg = ImVec2(0, 0));

	// Groups
	bool	BeginChild(const char* label, const ImVec2& size_arg = ImVec2(0, 0));
	void	EndChild();

	// Render
	void	RenderArrow(ImDrawList* draw_list, ImVec2 pos, ImU32 col, float sz, ImGuiDir direction);
	void	RenderCheckMark(ImDrawList* draw_list, ImVec2 pos, ImU32 col, float sz);

	// Helpers
	float	GetColorPickerWidth();
	float	CalcKeyBindWidth(const ImGuiKey& key);

	// Render
	void	DrawLabelShadow(ImDrawList* draw_list, ImVec2 pos, ImU32 col, const char* text);
}

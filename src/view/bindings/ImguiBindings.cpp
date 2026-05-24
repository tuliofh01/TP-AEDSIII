/**
 * @file ImguiBindings.cpp
 * @brief Bindings minimalistas de ImGui para Lua (4 cores).
 * @namespace project_view
 */
#include <lua.hpp>
#include <imgui.h>
#include <cstring>
#include <cstdarg>

/**
 * @namespace project_view
 * @brief Namespace flat para bindings de interface (GUI).
 */
namespace project_view {

lua_State* lState = nullptr;

// ========================================
// Bindings minimalistas (4 cores)
// ========================================

static int lua_SetNextWindowPos(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    const char* cond = luaL_optstring(L, 3, nullptr);
    ImGuiCond imCond = ImGuiCond_None;
    if (cond && strcmp(cond, "Always") == 0) imCond = ImGuiCond_Always;
    else if (cond && strcmp(cond, "Once") == 0) imCond = ImGuiCond_Once;
    else if (cond && strcmp(cond, "FirstUseEver") == 0) imCond = ImGuiCond_FirstUseEver;
    ImGui::SetNextWindowPos(ImVec2(x, y), imCond);
    return 0;
}

static int lua_SetNextWindowSize(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    const char* cond = luaL_optstring(L, 3, nullptr);
    ImGuiCond imCond = ImGuiCond_None;
    if (cond && strcmp(cond, "Always") == 0) imCond = ImGuiCond_Always;
    else if (cond && strcmp(cond, "Once") == 0) imCond = ImGuiCond_Once;
    else if (cond && strcmp(cond, "FirstUseEver") == 0) imCond = ImGuiCond_FirstUseEver;
    ImGui::SetNextWindowSize(ImVec2(x, y), imCond);
    return 0;
}

static int lua_Begin(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    bool* open = nullptr;
    if (lua_isboolean(L, 2)) {
        static bool windowOpen = true;
        if (!lua_toboolean(L, 2)) open = nullptr;
        else open = &windowOpen;
    }
    ImGuiWindowFlags flags = 0;
    if (lua_istable(L, 3)) {
        lua_pushnil(L);
        while (lua_next(L, 3) != 0) {
            const char* flag = lua_tostring(L, -1);
            if (strcmp(flag, "NoResize") == 0) flags |= ImGuiWindowFlags_NoResize;
            else if (strcmp(flag, "NoTitleBar") == 0) flags |= ImGuiWindowFlags_NoTitleBar;
            else if (strcmp(flag, "NoSavedSettings") == 0) flags |= ImGuiWindowFlags_NoSavedSettings;
            lua_pop(L, 1);
        }
    }
    bool result = ImGui::Begin(name, open, flags);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_End(lua_State* L) {
    (void)L;
    ImGui::End();
    return 0;
}

static int lua_Text(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    ImGui::Text("%s", text);
    return 0;
}

static int lua_Separator(lua_State* L) {
    (void)L;
    ImGui::Separator();
    return 0;
}

static int lua_Button(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    ImVec2 size(0, 0);
    if (lua_gettop(L) >= 2 && lua_isnumber(L, 2) && lua_isnumber(L, 3)) {
        size = ImVec2((float)lua_tonumber(L, 2), (float)lua_tonumber(L, 3));
    }
    bool result = ImGui::Button(label, size);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_InputText(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    static char buffer[256];
    if (lua_gettop(L) >= 2) {
        strncpy(buffer, luaL_checkstring(L, 2), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
    } else {
        buffer[0] = '\0';
    }
    bool result = ImGui::InputText(label, buffer, sizeof(buffer));
    lua_pushboolean(L, result);
    return 1;
}

static int lua_InputInt(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    int value = (int)luaL_optnumber(L, 2, 0);
    int step = (int)luaL_optnumber(L, 3, 1);
    int step_fast = (int)luaL_optnumber(L, 4, 100);
    bool result = ImGui::InputInt(label, &value, step, step_fast);
    lua_pushinteger(L, value);
    return 1;
}

static int lua_SameLine(lua_State* L) {
    float offset_from_pos_x = (float)luaL_optnumber(L, 1, 0.0);
    float spacing = (float)luaL_optnumber(L, 2, -1.0);
    ImGui::SameLine(offset_from_pos_x, spacing);
    return 0;
}

static int lua_Spacing(lua_State* L) {
    (void)L;
    ImGui::Spacing();
    return 0;
}

// ========================================
// Missing ImGui bindings
// ========================================

static int lua_OpenPopup(lua_State* L) {
    const char* str_id = luaL_checkstring(L, 1);
    ImGui::OpenPopup(str_id);
    return 0;
}

static int lua_CloseCurrentPopup(lua_State* L) {
    (void)L;
    ImGui::CloseCurrentPopup();
    return 0;
}

static int lua_BeginPopupModal(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    bool* open = nullptr;
    if (lua_isboolean(L, 2)) {
        if (!lua_toboolean(L, 2)) open = nullptr;
        else {
            static bool popupOpen = true;
            open = &popupOpen;
        }
    }
    ImGuiWindowFlags flags = 0;
    if (lua_istable(L, 3)) {
        lua_pushnil(L);
        while (lua_next(L, 3) != 0) {
            const char* flag = lua_tostring(L, -1);
            if (strcmp(flag, "NoResize") == 0) flags |= ImGuiWindowFlags_NoResize;
            else if (strcmp(flag, "NoTitleBar") == 0) flags |= ImGuiWindowFlags_NoTitleBar;
            else if (strcmp(flag, "NoSavedSettings") == 0) flags |= ImGuiWindowFlags_NoSavedSettings;
            lua_pop(L, 1);
        }
    }
    bool result = ImGui::BeginPopupModal(name, open, flags);
    lua_pushboolean(L, result);
    return 1;
}

static int lua_PushID(lua_State* L) {
    if (lua_isstring(L, 1)) {
        const char* str_id = luaL_checkstring(L, 1);
        ImGui::PushID(str_id);
    } else if (lua_isnumber(L, 1)) {
        int int_id = (int)luaL_checkinteger(L, 1);
        ImGui::PushID(int_id);
    } else if (lua_islightuserdata(L, 1)) {
        void* ptr_id = lua_touserdata(L, 1);
        ImGui::PushID(ptr_id);
    }
    return 0;
}

static int lua_PopID(lua_State* L) {
    (void)L;
    ImGui::PopID();
    return 0;
}

static int lua_PushStyleColor(lua_State* L) {
    int idx = (int)luaL_checknumber(L, 1);
    ImU32 col;
    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "r"); float r = (float)luaL_optnumber(L, -1, 1.0); lua_pop(L, 1);
        lua_getfield(L, 2, "g"); float g = (float)luaL_optnumber(L, -1, 1.0); lua_pop(L, 1);
        lua_getfield(L, 2, "b"); float b = (float)luaL_optnumber(L, -1, 1.0); lua_pop(L, 1);
        lua_getfield(L, 2, "a"); float a = (float)luaL_optnumber(L, -1, 1.0); lua_pop(L, 1);
        col = IM_COL32((int)(r*255), (int)(g*255), (int)(b*255), (int)(a*255));
    } else {
        col = (ImU32)luaL_checknumber(L, 2);
    }
    ImGui::PushStyleColor((ImGuiCol)idx, col);
    return 0;
}

static int lua_PopStyleColor(lua_State* L) {
    int count = (int)luaL_optnumber(L, 1, 1);
    ImGui::PopStyleColor(count);
    return 0;
}

// ========================================
// Tabela de funcoes (minimalista)
// ========================================
static const luaL_Reg imgui_funcs[] = {
    {"SetNextWindowPos", lua_SetNextWindowPos},
    {"SetNextWindowSize", lua_SetNextWindowSize},
    {"Begin", lua_Begin},
    {"End", lua_End},
    {"Text", lua_Text},
    {"Separator", lua_Separator},
    {"Button", lua_Button},
    {"InputText", lua_InputText},
    {"InputInt", lua_InputInt},
    {"SameLine", lua_SameLine},
    {"Spacing", lua_Spacing},
    {"PushStyleColor", lua_PushStyleColor},
    {"PopStyleColor", lua_PopStyleColor},
    {"OpenPopup", lua_OpenPopup},
    {"CloseCurrentPopup", lua_CloseCurrentPopup},
    {"BeginPopupModal", lua_BeginPopupModal},
    {"PushID", lua_PushID},
    {"PopID", lua_PopID},
    {nullptr, nullptr}
};

} // namespace project_view

// ========================================
// Funcao de carregamento (extern "C")
// ========================================
extern "C" void LoadImguiBindings() {
    using namespace project_view;
    if (!lState) return;
    lua_newtable(lState);
    luaL_setfuncs(lState, imgui_funcs, 0);
    lua_pushinteger(lState, ImGuiCol_Text); lua_setfield(lState, -2, "Col_Text");
    lua_pushinteger(lState, ImGuiCol_Button); lua_setfield(lState, -2, "Col_Button");
    lua_pushinteger(lState, ImGuiCol_ButtonHovered); lua_setfield(lState, -2, "Col_ButtonHovered");
    lua_pushinteger(lState, ImGuiCol_ButtonActive); lua_setfield(lState, -2, "Col_ButtonActive");
    lua_pushinteger(lState, ImGuiCol_FrameBg); lua_setfield(lState, -2, "Col_FrameBg");
    lua_pushinteger(lState, ImGuiCol_WindowBg); lua_setfield(lState, -2, "Col_WindowBg");
    lua_pushinteger(lState, ImGuiCol_Separator); lua_setfield(lState, -2, "Col_Separator");
    lua_setglobal(lState, "imgui");
}
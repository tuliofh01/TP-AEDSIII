/**
 * @file main.cpp
 * @brief Ponto de entrada do sistema AEDS III.
 * @brief GUI com hello_imgui + Lua (minimalista 4 cores).
 */
#include <iostream>
#include <filesystem>
#include <cstdint>
#include <optional>
#include "lua.hpp"
#include "luaaa.hpp"
#include "hello_imgui/hello_imgui.h"
#include "controller/DataManager.hpp"
#include "model/Record.hpp"

namespace project_view { extern lua_State* lState; }

namespace luaaa {
	template<>
	struct LuaStack<project_model::StudentRecord> {
		static void put(lua_State* L, const project_model::StudentRecord& rec) {
			lua_newtable(L);
			lua_pushstring(L, "id");
			lua_pushinteger(L, static_cast<lua_Integer>(rec.id));
			lua_settable(L, -3);
			lua_pushstring(L, "userId");
			lua_pushinteger(L, static_cast<lua_Integer>(rec.userId));
			lua_settable(L, -3);
			lua_pushstring(L, "name");
			lua_pushstring(L, rec.nameStr().c_str());
			lua_settable(L, -3);
			lua_pushstring(L, "birthDate");
			lua_pushinteger(L, static_cast<lua_Integer>(rec.birthDate));
			lua_settable(L, -3);
			lua_pushstring(L, "status");
			lua_pushinteger(L, static_cast<lua_Integer>(rec.status));
			lua_settable(L, -3);
		}
	};

	template<>
	struct LuaStack<std::optional<project_model::StudentRecord>> {
		static void put(lua_State* L, const std::optional<project_model::StudentRecord>& opt) {
			if (!opt.has_value()) {
				lua_pushnil(L);
				return;
			}
			LuaStack<project_model::StudentRecord>::put(L, opt.value());
		}
	};
}

extern "C" void LoadImguiBindings();

int main(int argc, char** argv) {
	(void)argc; (void)argv;

	// Cria estado Lua e carrega bibliotecas padrao
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	// Expoe estado global para bindings de ImGui
	project_view::lState = L;
	LoadImguiBindings();

	// Bind DataManager para Lua via luaaa
	using DM = project_controller::DataManager;
	project_controller::DataManager mgr;

	luaaa::LuaClass<DM> luaDM(L, "DataManager");
	luaDM.ctor();
	luaDM.fun("initialize", &DM::initialize);
	luaDM.fun("createStudent", &DM::createStudent);
	luaDM.fun("readStudent", &DM::readStudent);
	luaDM.fun("searchByName", &DM::searchByName);
	luaDM.fun("deleteStudent", &DM::deleteStudent);
	luaDM.fun("listAll", &DM::listAll);
	luaDM.fun("getLastError", &DM::getLastError);
	luaDM.fun("needsRebuild", &DM::needsRebuild);
	luaDM.fun("triggerRebuild", &DM::triggerRebuild);
	luaDM.fun("ignoreRebuildForSession", &DM::ignoreRebuildForSession);
	luaDM.fun("getNextDisplayId", &DM::getNextDisplayId);
	luaDM.fun("getActiveCount", &DM::getActiveCount);

	// Modulo global para acessar o manager
	luaaa::LuaModule(L).fun("getDataManager", [&]() -> DM* { return &mgr; });

	// Configura path Lua para encontrar views
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	std::string curPath = lua_tostring(L, -1);
	lua_pop(L, 1);
	lua_pushstring(L, (curPath + ";./src/view/scripts/views/?.lua;./src/view/scripts/views/?/init.lua").c_str());
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);

	// Carrega router.scripts
	if (luaL_dofile(L, "./src/view/scripts/views/router.lua") != LUA_OK) {
		std::cerr << "Lua Erro: " << lua_tostring(L, -1) << std::endl;
		lua_close(L);
		return -1;
	}

	// Inicializa arquivos de dados
	auto exeDir = std::filesystem::current_path();
	auto dataPath = (exeDir / "data" / "students.dat").string();
	std::filesystem::create_directories(exeDir / "data");
	(void)mgr.initialize(dataPath);

	// Configura e roda HelloImGui
	HelloImGui::RunnerParams runnerParams;
	runnerParams.appWindowParams.windowTitle = "AEDS III - Sistema de Matricula";
	runnerParams.appWindowParams.windowGeometry.size = { 800, 600 };

	// Tema minimalista 4 cores
	auto& theme = runnerParams.imGuiWindowParams;
	runnerParams.callbacks.ShowGui = [&]() {
		lua_getglobal(L, "RenderUI");
		if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
			std::cerr << "Lua Render Erro: " << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);
		}
	};

	HelloImGui::Run(runnerParams);
	lua_close(L);
	return 0;
}
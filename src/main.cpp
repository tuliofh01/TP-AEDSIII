#include <iostream>
#include <filesystem>
#include <cstdint>
#include <optional>
#include <vector>
#include <utility>
#include "lua.hpp"
#include "luaaa.hpp"
#include "hello_imgui/hello_imgui.h"
#include "controller/DataManager.hpp"
#include "model/Record.hpp"
#include "model/BPlusTree.hpp"

namespace project_view { extern lua_State* lState; }

namespace luaaa {

	// ---- StudentRecord -> Lua table ----
	template<>
	struct LuaStack<project_model::StudentRecord> {
		static void put(lua_State* L, const project_model::StudentRecord& rec) {
			lua_newtable(L);
			lua_pushstring(L, "id"); lua_pushinteger(L, rec.userId); lua_settable(L, -3);
			lua_pushstring(L, "name"); lua_pushstring(L, rec.nameStr().c_str()); lua_settable(L, -3);
			lua_pushstring(L, "email"); lua_pushstring(L, rec.emailStr().c_str()); lua_settable(L, -3);
			lua_pushstring(L, "cpf"); lua_pushstring(L, rec.cpfStr().c_str()); lua_settable(L, -3);
			lua_pushstring(L, "birthDate"); lua_pushinteger(L, rec.birthDate); lua_settable(L, -3);
			lua_pushstring(L, "course"); lua_pushstring(L, rec.courseStr().c_str()); lua_settable(L, -3);
			lua_pushstring(L, "enrollmentYear"); lua_pushinteger(L, rec.enrollmentYear); lua_settable(L, -3);
			lua_pushstring(L, "status"); lua_pushinteger(L, rec.status); lua_settable(L, -3);
		}
	};

	template<>
	struct LuaStack<std::optional<project_model::StudentRecord>> {
		static void put(lua_State* L, const std::optional<project_model::StudentRecord>& opt) {
			if (!opt.has_value()) { lua_pushnil(L); return; }
			LuaStack<project_model::StudentRecord>::put(L, opt.value());
		}
	};

	template<>
	struct LuaStack<std::vector<project_model::StudentRecord>> {
		static void put(lua_State* L, const std::vector<project_model::StudentRecord>& vec) {
			lua_newtable(L);
			for (size_t i = 0; i < vec.size(); ++i) {
				lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
				LuaStack<project_model::StudentRecord>::put(L, vec[i]);
				lua_settable(L, -3);
			}
		}
	};

	// ---- TeacherRecord -> Lua table ----
	template<>
	struct LuaStack<project_model::TeacherRecord> {
		static void put(lua_State* L, const project_model::TeacherRecord& rec) {
			lua_newtable(L);
			lua_pushstring(L, "id"); lua_pushinteger(L, rec.userId); lua_settable(L, -3);
			lua_pushstring(L, "name"); lua_pushstring(L, rec.nameStr().c_str()); lua_settable(L, -3);
			lua_pushstring(L, "email"); lua_pushstring(L, rec.emailStr().c_str()); lua_settable(L, -3);
			lua_pushstring(L, "cpf"); lua_pushstring(L, rec.cpfStr().c_str()); lua_settable(L, -3);
			lua_pushstring(L, "department"); lua_pushstring(L, rec.deptStr().c_str()); lua_settable(L, -3);
			lua_pushstring(L, "specialization"); lua_pushstring(L, rec.specStr().c_str()); lua_settable(L, -3);
			lua_pushstring(L, "hireDate"); lua_pushinteger(L, rec.hireDate); lua_settable(L, -3);
			lua_pushstring(L, "status"); lua_pushinteger(L, rec.status); lua_settable(L, -3);
		}
	};

	template<>
	struct LuaStack<std::optional<project_model::TeacherRecord>> {
		static void put(lua_State* L, const std::optional<project_model::TeacherRecord>& opt) {
			if (!opt.has_value()) { lua_pushnil(L); return; }
			LuaStack<project_model::TeacherRecord>::put(L, opt.value());
		}
	};

	template<>
	struct LuaStack<std::vector<project_model::TeacherRecord>> {
		static void put(lua_State* L, const std::vector<project_model::TeacherRecord>& vec) {
			lua_newtable(L);
			for (size_t i = 0; i < vec.size(); ++i) {
				lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
				LuaStack<project_model::TeacherRecord>::put(L, vec[i]);
				lua_settable(L, -3);
			}
		}
	};

	// ---- SubjectRecord -> Lua table ----
	template<>
	struct LuaStack<project_model::SubjectRecord> {
		static void put(lua_State* L, const project_model::SubjectRecord& rec) {
			lua_newtable(L);
			lua_pushstring(L, "id"); lua_pushinteger(L, rec.subjectId); lua_settable(L, -3);
			lua_pushstring(L, "name"); lua_pushstring(L, rec.nameStr().c_str()); lua_settable(L, -3);
			lua_pushstring(L, "code"); lua_pushstring(L, rec.codeStr().c_str()); lua_settable(L, -3);
			lua_pushstring(L, "credits"); lua_pushinteger(L, rec.credits); lua_settable(L, -3);
			lua_pushstring(L, "teacherId"); lua_pushinteger(L, rec.teacherId); lua_settable(L, -3);
			lua_pushstring(L, "status"); lua_pushinteger(L, rec.status); lua_settable(L, -3);
		}
	};

	template<>
	struct LuaStack<std::optional<project_model::SubjectRecord>> {
		static void put(lua_State* L, const std::optional<project_model::SubjectRecord>& opt) {
			if (!opt.has_value()) { lua_pushnil(L); return; }
			LuaStack<project_model::SubjectRecord>::put(L, opt.value());
		}
	};

	template<>
	struct LuaStack<std::vector<project_model::SubjectRecord>> {
		static void put(lua_State* L, const std::vector<project_model::SubjectRecord>& vec) {
			lua_newtable(L);
			for (size_t i = 0; i < vec.size(); ++i) {
				lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
				LuaStack<project_model::SubjectRecord>::put(L, vec[i]);
				lua_settable(L, -3);
			}
		}
	};

	// ---- BTreeLeafValue (enrollment) -> Lua table ----
	template<>
	struct LuaStack<project_model::BTreeLeafValue> {
		static void put(lua_State* L, const project_model::BTreeLeafValue& val) {
			lua_newtable(L);
			lua_pushstring(L, "studentId"); lua_pushinteger(L, val.studentId); lua_settable(L, -3);
			lua_pushstring(L, "subjectId"); lua_pushinteger(L, val.subjectId); lua_settable(L, -3);
			lua_pushstring(L, "teacherId"); lua_pushinteger(L, val.teacherId); lua_settable(L, -3);
			lua_pushstring(L, "grade"); lua_pushnumber(L, static_cast<lua_Number>(val.grade)); lua_settable(L, -3);
			lua_pushstring(L, "semester"); lua_pushstring(L, val.semesterStr().c_str()); lua_settable(L, -3);
		}
	};

	template<>
	struct LuaStack<std::optional<project_model::BTreeLeafValue>> {
		static void put(lua_State* L, const std::optional<project_model::BTreeLeafValue>& opt) {
			if (opt.has_value()) {
				LuaStack<project_model::BTreeLeafValue>::put(L, opt.value());
			} else {
				lua_pushnil(L);
			}
		}
	};

	template<>
	struct LuaStack<std::vector<project_model::BTreeLeafValue>> {
		static void put(lua_State* L, const std::vector<project_model::BTreeLeafValue>& vec) {
			lua_newtable(L);
			for (size_t i = 0; i < vec.size(); ++i) {
				lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
				LuaStack<project_model::BTreeLeafValue>::put(L, vec[i]);
				lua_settable(L, -3);
			}
		}
	};
}

extern "C" void LoadImguiBindings();

int main(int argc, char** argv) {
	(void)argc; (void)argv;

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	project_view::lState = L;
	LoadImguiBindings();

	using DM = project_controller::DataManager;
	project_controller::DataManager mgr;

	luaaa::LuaClass<DM> luaDM(L, "DataManager");
	luaDM.ctor();

	// Initialize
	luaDM.fun("initialize", &DM::initialize);

	// Student
	luaDM.fun("createStudent", &DM::createStudent);
	luaDM.fun("readStudent", &DM::readStudent);
	luaDM.fun("deleteStudent", &DM::deleteStudent);
	luaDM.fun("listAllStudents", &DM::listAllStudents);
	luaDM.fun("getNextStudentId", &DM::getNextStudentId);

	// Teacher
	luaDM.fun("createTeacher", &DM::createTeacher);
	luaDM.fun("readTeacher", &DM::readTeacher);
	luaDM.fun("deleteTeacher", &DM::deleteTeacher);
	luaDM.fun("listAllTeachers", &DM::listAllTeachers);
	luaDM.fun("getNextTeacherId", &DM::getNextTeacherId);

	// Subject
	luaDM.fun("createSubject", &DM::createSubject);
	luaDM.fun("readSubject", &DM::readSubject);
	luaDM.fun("deleteSubject", &DM::deleteSubject);
	luaDM.fun("listAllSubjects", &DM::listAllSubjects);
	luaDM.fun("getNextSubjectId", &DM::getNextSubjectId);

	// Enrollment
	luaDM.fun("enrollStudent", &DM::enrollStudent);
	luaDM.fun("getEnrollment", &DM::getEnrollment);
	luaDM.fun("updateGrade", &DM::updateGrade);
	luaDM.fun("unenroll", &DM::unenroll);
	luaDM.fun("getEnrollmentsByStudent", &DM::getEnrollmentsByStudent);
	luaDM.fun("getEnrollmentsBySubject", &DM::getEnrollmentsBySubject);

	// Info
	luaDM.fun("getLastError", &DM::getLastError);
	luaDM.fun("getActiveCount", &DM::getActiveCount);

	// Expose manager as global
	luaaa::LuaModule(L).fun("getDataManager", [&]() -> DM* { return &mgr; });

	// Set up Lua path
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	std::string curPath = lua_tostring(L, -1);
	lua_pop(L, 1);
	lua_pushstring(L, (curPath + ";./src/view/scripts/views/?.lua;./src/view/scripts/views/?/init.lua").c_str());
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);

	// Load router
	if (luaL_dofile(L, "./src/view/scripts/views/router.lua") != LUA_OK) {
		std::cerr << "Lua Erro: " << lua_tostring(L, -1) << std::endl;
		lua_close(L);
		return -1;
	}

	// Initialize data
	auto exeDir = std::filesystem::current_path();
	auto dataDir = (exeDir / "data").string();
	std::filesystem::create_directories(dataDir);
	(void)mgr.initialize(dataDir);

	// Configure HelloImGui
	HelloImGui::RunnerParams runnerParams;
	runnerParams.appWindowParams.windowTitle = "AEDS III - Sistema de Matricula";
	runnerParams.appWindowParams.windowGeometry.size = { 800, 600 };

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

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
#include "view/ScriptArchive.hpp"

namespace project_view { extern lua_State* lState; }

namespace luaaa {

	// ---- StudentRecord -> Lua table (used by Student model delegation) ----
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

	// ---- Student model -> delegates to StudentRecord ----
	template<>
	struct LuaStack<project_model::Student> {
		static void put(lua_State* L, const project_model::Student& s) {
			LuaStack<project_model::StudentRecord>::put(L, s.raw());
		}
	};

	template<>
	struct LuaStack<std::optional<project_model::Student>> {
		static void put(lua_State* L, const std::optional<project_model::Student>& opt) {
			if (!opt.has_value()) { lua_pushnil(L); return; }
			LuaStack<project_model::Student>::put(L, opt.value());
		}
	};

	template<>
	struct LuaStack<std::vector<project_model::Student>> {
		static void put(lua_State* L, const std::vector<project_model::Student>& vec) {
			lua_newtable(L);
			auto idx = size_t{1};
			for (const auto& s : vec) {
				lua_pushinteger(L, static_cast<lua_Integer>(idx++));
				LuaStack<project_model::Student>::put(L, s);
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
	struct LuaStack<project_model::Teacher> {
		static void put(lua_State* L, const project_model::Teacher& t) {
			LuaStack<project_model::TeacherRecord>::put(L, t.raw());
		}
	};

	template<>
	struct LuaStack<std::optional<project_model::Teacher>> {
		static void put(lua_State* L, const std::optional<project_model::Teacher>& opt) {
			if (!opt.has_value()) { lua_pushnil(L); return; }
			LuaStack<project_model::Teacher>::put(L, opt.value());
		}
	};

	template<>
	struct LuaStack<std::vector<project_model::Teacher>> {
		static void put(lua_State* L, const std::vector<project_model::Teacher>& vec) {
			lua_newtable(L);
			auto idx = size_t{1};
			for (const auto& t : vec) {
				lua_pushinteger(L, static_cast<lua_Integer>(idx++));
				LuaStack<project_model::Teacher>::put(L, t);
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
	struct LuaStack<project_model::Subject> {
		static void put(lua_State* L, const project_model::Subject& s) {
			LuaStack<project_model::SubjectRecord>::put(L, s.raw());
		}
	};

	template<>
	struct LuaStack<std::optional<project_model::Subject>> {
		static void put(lua_State* L, const std::optional<project_model::Subject>& opt) {
			if (!opt.has_value()) { lua_pushnil(L); return; }
			LuaStack<project_model::Subject>::put(L, opt.value());
		}
	};

	template<>
	struct LuaStack<std::vector<project_model::Subject>> {
		static void put(lua_State* L, const std::vector<project_model::Subject>& vec) {
			lua_newtable(L);
			auto idx = size_t{1};
			for (const auto& s : vec) {
				lua_pushinteger(L, static_cast<lua_Integer>(idx++));
				LuaStack<project_model::Subject>::put(L, s);
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
			lua_pushstring(L, "grade"); lua_pushinteger(L, static_cast<lua_Integer>(val.grade)); lua_settable(L, -3);
			lua_pushstring(L, "approved"); lua_pushboolean(L, val.isApproved() ? 1 : 0); lua_settable(L, -3);
			lua_pushstring(L, "semester"); lua_pushstring(L, val.semesterStr().c_str()); lua_settable(L, -3);
		}
	};

	template<>
	struct LuaStack<project_model::Enrollment> {
		static void put(lua_State* L, const project_model::Enrollment& e) {
			LuaStack<project_model::BTreeLeafValue>::put(L, e.raw());
		}
	};

	template<>
	struct LuaStack<std::optional<project_model::Enrollment>> {
		static void put(lua_State* L, const std::optional<project_model::Enrollment>& opt) {
			if (opt.has_value()) {
				LuaStack<project_model::Enrollment>::put(L, opt.value());
			} else {
				lua_pushnil(L);
			}
		}
	};

	template<>
	struct LuaStack<std::vector<project_model::Enrollment>> {
		static void put(lua_State* L, const std::vector<project_model::Enrollment>& vec) {
			lua_newtable(L);
			auto idx = size_t{1};
			for (const auto& e : vec) {
				lua_pushinteger(L, static_cast<lua_Integer>(idx++));
				LuaStack<project_model::Enrollment>::put(L, e);
				lua_settable(L, -3);
			}
		}
	};

	// ---- LoginResult -> Lua table ----
	template<>
	struct LuaStack<project_controller::LoginResult> {
		static void put(lua_State* L, const project_controller::LoginResult& r) {
			lua_newtable(L);
			lua_pushstring(L, "userId"); lua_pushinteger(L, r.userId); lua_settable(L, -3);
			lua_pushstring(L, "role"); lua_pushlstring(L, &r.role, 1); lua_settable(L, -3);
			lua_pushstring(L, "name"); lua_pushstring(L, r.name.c_str()); lua_settable(L, -3);
		}
	};

	template<>
	struct LuaStack<std::optional<project_controller::LoginResult>> {
		static void put(lua_State* L,
			const std::optional<project_controller::LoginResult>& opt) {
			if (opt.has_value()) {
				LuaStack<project_controller::LoginResult>::put(L, opt.value());
			} else {
				lua_pushnil(L);
			}
		}
	};

}

extern "C" void LoadImguiBindings();

int main(int argc, char** argv) {
	lua_State* L = luaL_newstate();

	std::ignore = argc;
	luaL_openlibs(L);

	project_view::lState = L;
	LoadImguiBindings();

	using DM = project_controller::DataManager;
	auto mgr = project_controller::DataManager{};

	// Register DataManager methods in a Lua module table (LuaModule).
	// LuaModule uses NonMemberFunctionCaller (skip=0), avoiding the
	// LuaClass MemberFunctionCaller bug where skip=1 shifts args down.
	luaaa::LuaModule dmMethods(L, "dm_methods");

	dmMethods.fun("initialize", [](DM* dm, const std::string& dir) { return dm->initialize(dir); });

	dmMethods.fun("createStudent", [](DM* dm, const std::string& name, const std::string& email,
		const std::string& cpf, int password, int birthDate,
		const std::string& courseName, int enrollmentYear) {
		return dm->createStudent(name, email, cpf, static_cast<uint16_t>(password),
			static_cast<uint32_t>(birthDate), courseName, enrollmentYear);
	});
	dmMethods.fun("readStudent", [](DM* dm, int32_t id) { return dm->readStudent(id); });
	dmMethods.fun("deleteStudent", [](DM* dm, int32_t id) { return dm->deleteStudent(id); });
	dmMethods.fun("listAllStudents", [](DM* dm) { return dm->listAllStudents(); });
	dmMethods.fun("getNextStudentId", [](DM* dm) { return dm->getNextStudentId(); });

	dmMethods.fun("createTeacher", [](DM* dm, const std::string& name, const std::string& email,
		const std::string& cpf, int password, const std::string& department,
		const std::string& specialization, int hireDate) {
		return dm->createTeacher(name, email, cpf, static_cast<uint16_t>(password),
			department, specialization, static_cast<uint32_t>(hireDate));
	});
	dmMethods.fun("readTeacher", [](DM* dm, int32_t id) { return dm->readTeacher(id); });
	dmMethods.fun("deleteTeacher", [](DM* dm, int32_t id) { return dm->deleteTeacher(id); });
	dmMethods.fun("listAllTeachers", [](DM* dm) { return dm->listAllTeachers(); });
	dmMethods.fun("getNextTeacherId", [](DM* dm) { return dm->getNextTeacherId(); });

	dmMethods.fun("createSubject", [](DM* dm, const std::string& name, const std::string& code,
		int32_t credits, int32_t teacherId) {
		return dm->createSubject(name, code, credits, teacherId);
	});
	dmMethods.fun("readSubject", [](DM* dm, int32_t id) { return dm->readSubject(id); });
	dmMethods.fun("deleteSubject", [](DM* dm, int32_t id) { return dm->deleteSubject(id); });
	dmMethods.fun("listAllSubjects", [](DM* dm) { return dm->listAllSubjects(); });
	dmMethods.fun("getNextSubjectId", [](DM* dm) { return dm->getNextSubjectId(); });

	dmMethods.fun("enrollStudent", [](DM* dm, int32_t studentId, int32_t subjectId,
		int32_t teacherId, const std::string& semester) {
		return dm->enrollStudent(studentId, subjectId, teacherId, semester);
	});
	dmMethods.fun("getEnrollment", [](DM* dm, int32_t studentId, int32_t subjectId) {
		return dm->getEnrollment(studentId, subjectId);
	});
	dmMethods.fun("updateGrade", [](DM* dm, int32_t studentId, int32_t subjectId, int grade) {
		return dm->updateGrade(studentId, subjectId, static_cast<uint8_t>(grade));
	});
	dmMethods.fun("unenroll", [](DM* dm, int32_t studentId, int32_t subjectId) {
		return dm->unenroll(studentId, subjectId);
	});
	dmMethods.fun("getEnrollmentsByStudent", [](DM* dm, int32_t studentId) {
		return dm->getEnrollmentsByStudent(studentId);
	});
	dmMethods.fun("getEnrollmentsBySubject", [](DM* dm, int32_t subjectId) {
		return dm->getEnrollmentsBySubject(subjectId);
	});

	dmMethods.fun("login", [](DM* dm, const std::string& cpf, int password) {
		return dm->login(cpf, static_cast<uint16_t>(password));
	});

	dmMethods.fun("getLastError", [](DM* dm) { return dm->getLastError(); });
	dmMethods.fun("getActiveCount", [](DM* dm, int type) { return dm->getActiveCount(static_cast<char>(type)); });

	// Push DataManager as a Lua light userdata with a metatable whose __index
	// is the dm_methods module table. Lua method calls (dm:method()) go through
	// __index → NonMemberFunctionCaller (skip=0), so DM* is at stack position 1.
	lua_pushlightuserdata(L, &mgr);
	lua_newtable(L);                                                         // metatable
	lua_getglobal(L, "dm_methods");                                          // push module table
	lua_setfield(L, -2, "__index");                                          // metatable.__index = dm_methods
	lua_setmetatable(L, -2);                                                 // set metatable on light userdata
	lua_setglobal(L, "dm");

	// getDataManager() → returns the dm global
	lua_pushcfunction(L, [](lua_State* ls) -> int {
		lua_getglobal(ls, "dm");
		return 1;
	});
	lua_setglobal(L, "getDataManager");

	// Load scripts from binary archive
	auto exePath = std::filesystem::path(argv[0]);
	auto scriptsFile = exePath.parent_path() / "scripts.bin";
	if (!std::filesystem::exists(scriptsFile)) {
		scriptsFile = std::filesystem::current_path() / "builds" / "compiled_executable" / "scripts.bin";
	}

	auto archive = project_view::ScriptArchive{};
	if (!std::filesystem::exists(scriptsFile) || !archive.load(scriptsFile.string())) {
		std::cerr << "Erro: scripts.bin não encontrado em "
				  << (exePath.parent_path() / "scripts.bin").string() << std::endl;
		lua_close(L);
		return -1;
	}

	archive.registerAll(L);

	// Initialize data
	auto dataDir = (std::filesystem::current_path() / "data").string();
	std::filesystem::create_directories(dataDir);
	std::ignore = mgr.initialize(dataDir);

	// Populate sample data for fresh or incomplete databases
	if (mgr.getActiveCount('T') == 0 || mgr.getActiveCount('B') == 0) {
		lua_getglobal(L, "require");
		lua_pushstring(L, "misc.populate_samples");
		if (lua_pcall(L, 1, 1, 0) == LUA_OK) {
			lua_getfield(L, -1, "populate");
			lua_getglobal(L, "dm");
			if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
				std::cerr << "Erro ao popular amostras: " << lua_tostring(L, -1) << std::endl;
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		} else {
			std::cerr << "Erro ao carregar populate_samples: " << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);
		}
	}

	// Configure HelloImGui
	auto runnerParams = HelloImGui::RunnerParams{};
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

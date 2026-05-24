#include <iostream>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <functional>
#include <sstream>

#include "controller/DataManager.hpp"

#define TEST_DIR "data/test"

static void cleanup() {
	if (std::filesystem::exists(TEST_DIR))
		std::filesystem::remove_all(TEST_DIR);
}

class TestRunner {
	int passed_ = 0;
	int failed_ = 0;
public:
	void run(const char* name, std::function<void()> test) {
		try {
			test();
			std::cout << "[PASS] " << name << '\n';
			++passed_;
		} catch (const std::exception& e) {
			std::cerr << "[FAIL] " << name << ": " << e.what() << '\n';
			++failed_;
		} catch (...) {
			std::cerr << "[FAIL] " << name << ": unknown exception\n";
			++failed_;
		}
	}
	int getFailed() const { return failed_; }
	void summary() {
		std::cout << "\n============================\n";
		std::cout << "Total: " << (passed_ + failed_)
			<< " | Pass: " << passed_
			<< " | Fail: " << failed_ << '\n';
		std::cout << "============================\n";
		if (failed_ == 0)
			std::cout << "ALL TESTS PASSED!\n";
		else
			std::cerr << "SOME TESTS FAILED!\n";
	}
};

static TestRunner runner;

static void test_initialize() {
	cleanup();
	project_controller::DataManager dm;
	assert(dm.initialize(TEST_DIR) && "Failed to initialize");
}

static void test_create_student() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DIR);
	bool ok = dm.createStudent("Joao Silva", "joao@email.com", "12345678901", 15051990, "Ciencia Comp", 2026);
	assert(ok && "Failed to create student");
	assert(dm.getActiveCount('S') == 1 && "Active count should be 1");
}

static void test_read_student() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DIR);
	(void)dm.createStudent("Maria Santos", "maria@email.com", "98765432101", 20101995, "Engenharia", 2025);
	auto rec = dm.readStudent(1);
	assert(rec.has_value() && "Failed to read student");
	assert(std::strcmp(rec->name, "Maria Santos") == 0 && "Wrong name");
}

static void test_read_nonexistent() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DIR);
	auto rec = dm.readStudent(999);
	assert(!rec.has_value() && "Should not find student");
}

static void test_list_all_students() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DIR);
	(void)dm.createStudent("Aluno A", "a@a.com", "1", 10101010, "Curso", 2026);
	(void)dm.createStudent("Aluno B", "b@b.com", "2", 20202020, "Curso", 2026);
	(void)dm.createStudent("Aluno C", "c@c.com", "3", 30303030, "Curso", 2026);
	auto list = dm.listAllStudents();
	assert(list.size() == 3 && "Should have 3 students");
}

static void test_soft_delete() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DIR);
	(void)dm.createStudent("Para Deletar", "del@del.com", "0", 11111111, "Curso", 2026);
	bool ok = dm.deleteStudent(1);
	assert(ok && "Failed to delete");
	assert(dm.getActiveCount('S') == 0 && "Should have 0 active");
}

static void test_create_teacher_and_subject() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DIR);
	bool tOk = dm.createTeacher("Prof Silva", "silva@uni.com", "11122233344", "DCC", "BD", 10032020);
	assert(tOk && "Failed to create teacher");
	bool sOk = dm.createSubject("BD II", "BDI002", 60, 1);
	assert(sOk && "Failed to create subject");
	auto sub = dm.readSubject(1);
	assert(sub.has_value() && "Should read subject");
	assert(std::strcmp(sub->name, "BD II") == 0 && "Wrong subject name");
}

static void test_enrollment() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DIR);
	(void)dm.createStudent("Aluno", "a@a.com", "1", 10101010, "Curso", 2026);
	(void)dm.createTeacher("Prof", "p@u.com", "2", "DCC", "BD", 10032020);
	(void)dm.createSubject("BD", "BD001", 60, 1);
	bool enr = dm.enrollStudent(1, 1, 1, "2026-1");
	assert(enr && "Failed to enroll");
	auto e = dm.getEnrollment(1, 1);
	assert(e.has_value() && "Should find enrollment");
	assert(std::strcmp(e->semesterStr().c_str(), "2026-1") == 0 && "Wrong semester");
}

static void test_update_grade() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DIR);
	(void)dm.createStudent("Aluno", "a@a.com", "1", 10101010, "Curso", 2026);
	(void)dm.createTeacher("Prof", "p@u.com", "2", "DCC", "BD", 10032020);
	(void)dm.createSubject("BD", "BD001", 60, 1);
	(void)dm.enrollStudent(1, 1, 1, "2026-1");
	bool up = dm.updateGrade(1, 1, 8.5f);
	assert(up && "Failed to update grade");
	auto e = dm.getEnrollment(1, 1);
	assert(e.has_value() && e->grade == 8.5f && "Grade mismatch");
}

static void test_enrollments_by_student() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DIR);
	(void)dm.createStudent("Aluno", "a@a.com", "1", 10101010, "Curso", 2026);
	(void)dm.createTeacher("Prof", "p@u.com", "2", "DCC", "BD", 10032020);
	(void)dm.createSubject("BD", "BD001", 60, 1);
	(void)dm.createSubject("LP", "LP001", 60, 1);
	(void)dm.enrollStudent(1, 1, 1, "2026-1");
	(void)dm.enrollStudent(1, 2, 1, "2026-1");
	auto list = dm.getEnrollmentsByStudent(1);
	assert(list.size() == 2 && "Should have 2 enrollments");
}

int main() {
	std::cout << "=== DataManager Tests ===\n\n";

	runner.run("test_initialize", test_initialize);
	runner.run("test_create_student", test_create_student);
	runner.run("test_read_student", test_read_student);
	runner.run("test_read_nonexistent", test_read_nonexistent);
	runner.run("test_list_all_students", test_list_all_students);
	runner.run("test_soft_delete", test_soft_delete);
	runner.run("test_create_teacher_and_subject", test_create_teacher_and_subject);
	runner.run("test_enrollment", test_enrollment);
	runner.run("test_update_grade", test_update_grade);
	runner.run("test_enrollments_by_student", test_enrollments_by_student);

	runner.summary();
	return runner.getFailed() > 0 ? 1 : 0;
}

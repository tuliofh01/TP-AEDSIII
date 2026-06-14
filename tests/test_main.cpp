// Unit tests for DataManager CRUD and enrollment operations.
// Each test cleans up the test directory and starts fresh.

#include <iostream>
#include <cassert>
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
			std::cerr << "[PASS] " << name << '\n';
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
		std::cerr << "\n============================\n";
		std::cerr << "Total: " << (passed_ + failed_)
			<< " | Pass: " << passed_
			<< " | Fail: " << failed_ << '\n';
		std::cerr << "============================\n";
		if (failed_ == 0)
			std::cerr << "ALL TESTS PASSED!\n";
		else
			std::cerr << "SOME TESTS FAILED!\n";
	}
};

static TestRunner runner;

// Test: basic DataManager initialization and directory creation
static void test_initialize() {
	cleanup();
	auto dm = project_controller::DataManager{};
	assert(dm.initialize(TEST_DIR) && "Failed to initialize");
}

// Test: create a student and verify active count
static void test_create_student() {
	cleanup();
	auto dm = project_controller::DataManager{};
	(void)dm.initialize(TEST_DIR);
	bool ok = dm.createStudent("Joao Silva", "joao@email.com", "12345678901", 1234, 15051990, "Ciencia Comp", 2026);
	assert(ok && "Failed to create student");
	assert(dm.getActiveCount('S') == 1 && "Active count should be 1");
}

// Test: read a created student and verify field values
static void test_read_student() {
	cleanup();
	auto dm = project_controller::DataManager{};
	(void)dm.initialize(TEST_DIR);
	(void)dm.createStudent("Maria Santos", "maria@email.com", "98765432101", 1234, 20101995, "Engenharia", 2025);
	auto rec = dm.readStudent(1);
	assert(rec.has_value() && "Failed to read student");
	assert(rec->getName() == "Maria Santos" && "Wrong name");
}

// Test: reading a non-existent ID returns nullopt
static void test_read_nonexistent() {
	cleanup();
	auto dm = project_controller::DataManager{};
	(void)dm.initialize(TEST_DIR);
	auto rec = dm.readStudent(999);
	assert(!rec.has_value() && "Should not find student");
}

// Test: listing all students returns the correct count
static void test_list_all_students() {
	cleanup();
	auto dm = project_controller::DataManager{};
	(void)dm.initialize(TEST_DIR);
	(void)dm.createStudent("Aluno A", "a@a.com", "1", 1234, 10101010, "Curso", 2026);
	(void)dm.createStudent("Aluno B", "b@b.com", "2", 1234, 20202020, "Curso", 2026);
	(void)dm.createStudent("Aluno C", "c@c.com", "3", 1234, 30303030, "Curso", 2026);
	auto list = dm.listAllStudents();
	assert(list.size() == 3 && "Should have 3 students");
}

// Test: soft-delete a student and verify active count drops
static void test_soft_delete() {
	cleanup();
	auto dm = project_controller::DataManager{};
	(void)dm.initialize(TEST_DIR);
	(void)dm.createStudent("Para Deletar", "del@del.com", "0", 1234, 11111111, "Curso", 2026);
	bool ok = dm.deleteStudent(1);
	assert(ok && "Failed to delete");
	assert(dm.getActiveCount('S') == 0 && "Should have 0 active");
}

// Test: create a teacher and a subject, then read the subject back
static void test_create_teacher_and_subject() {
	cleanup();
	auto dm = project_controller::DataManager{};
	(void)dm.initialize(TEST_DIR);
	bool tOk = dm.createTeacher("Prof Silva", "silva@uni.com", "11122233344", 1234, "DCC", "BD", 10032020);
	assert(tOk && "Failed to create teacher");
	bool sOk = dm.createSubject("BD II", "BDI002", 60, 1);
	assert(sOk && "Failed to create subject");
	auto sub = dm.readSubject(1);
	assert(sub.has_value() && "Should read subject");
	assert(sub->getName() == "BD II" && "Wrong subject name");
}

// Test: enroll a student in a subject and verify the enrollment record
static void test_enrollment() {
	cleanup();
	auto dm = project_controller::DataManager{};
	(void)dm.initialize(TEST_DIR);
	(void)dm.createStudent("Aluno", "a@a.com", "1", 1234, 10101010, "Curso", 2026);
	(void)dm.createTeacher("Prof", "p@u.com", "2", 1234, "DCC", "BD", 10032020);
	(void)dm.createSubject("BD", "BD001", 60, 1);
	bool enr = dm.enrollStudent(1, 1, 1, "2026-1");
	assert(enr && "Failed to enroll");
	auto e = dm.getEnrollment(1, 1);
	assert(e.has_value() && "Should find enrollment");
	assert(e->getSemester() == "2026-1" && "Wrong semester");
}

// Test: update a student's grade in an enrollment
static void test_update_grade() {
	cleanup();
	auto dm = project_controller::DataManager{};
	(void)dm.initialize(TEST_DIR);
	(void)dm.createStudent("Aluno", "a@a.com", "1", 1234, 10101010, "Curso", 2026);
	(void)dm.createTeacher("Prof", "p@u.com", "2", 1234, "DCC", "BD", 10032020);
	(void)dm.enrollStudent(1, 1, 1, "2026-1");
	bool up = dm.updateGrade(1, 1, 85);
	assert(up && "Failed to update grade");
	auto e = dm.getEnrollment(1, 1);
	assert(e.has_value() && e->getGrade() == 85 && "Grade mismatch");
}

// Test: login with correct/wrong credentials for both roles
static void test_login() {
	cleanup();
	auto dm = project_controller::DataManager{};
	(void)dm.initialize(TEST_DIR);
	// Create teacher with known CPF+password
	(void)dm.createTeacher("Prof Teste", "prof@t.com", "00000000000", 4321, "DCC", "BD", 20200101);
	// Create student with known CPF+password
	(void)dm.createStudent("Aluno Teste", "aluno@t.com", "11111111111", 1234, 15051990, "CC", 2026);

	// Login as teacher
	auto t = dm.login("00000000000", 4321);
	assert(t.has_value() && "Teacher login should succeed");
	assert(t->role == 'T' && "Should be teacher role");
	assert(t->name == "Prof Teste" && "Wrong teacher name");

	// Login as student
	auto s = dm.login("11111111111", 1234);
	assert(s.has_value() && "Student login should succeed");
	assert(s->role == 'S' && "Should be student role");
	assert(s->name == "Aluno Teste" && "Wrong student name");

	// Wrong password
	assert(!dm.login("00000000000", 9999).has_value() && "Wrong password should fail");
	// Nonexistent CPF
	assert(!dm.login("99999999999", 1234).has_value() && "Nonexistent CPF should fail");
}

// Test: retrieve all enrollments for a given student
static void test_enrollments_by_student() {
	cleanup();
	auto dm = project_controller::DataManager{};
	(void)dm.initialize(TEST_DIR);
	(void)dm.createStudent("Aluno", "a@a.com", "1", 1234, 10101010, "Curso", 2026);
	(void)dm.createTeacher("Prof", "p@u.com", "2", 1234, "DCC", "BD", 10032020);
	(void)dm.createSubject("BD", "BD001", 60, 1);
	(void)dm.createSubject("LP", "LP001", 60, 1);
	(void)dm.enrollStudent(1, 1, 1, "2026-1");
	(void)dm.enrollStudent(1, 2, 1, "2026-1");
	auto list = dm.getEnrollmentsByStudent(1);
	assert(list.size() == 2 && "Should have 2 enrollments");
}

// Runs all tests sequentially and prints a summary.
int main() {
	std::ios::sync_with_stdio(true);
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;
	std::cerr << "=== DataManager Tests ===\n\n";

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
	runner.run("test_login", test_login);

	runner.summary();
	return runner.getFailed() > 0 ? 1 : 0;
}

/**
 * @file test_main.cpp
 * @brief Testes unitarios para o DataManager (Arquitetura MVC).
 */
#include <iostream>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <functional>
#include <sstream>

#include "controller/DataManager.hpp"

#define TEST_DATA_FILE "data/students_test.dat"

// ========================================
// TestRunner - framework minimalista
// ========================================
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
			std::cerr << "[FAIL] " << name << ": excecao desconhecida\n";
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
			std::cout << "TODOS OS TESTES PASSARAM!\n";
		else
			std::cerr << "ALGUNS TESTES FALHARAM!\n";
	}
};

static TestRunner runner;

// Limpa arquivo de teste
static void cleanup() {
	if (std::filesystem::exists(TEST_DATA_FILE))
		std::filesystem::remove(TEST_DATA_FILE);
	auto idxPath = std::string(TEST_DATA_FILE).replace(
		std::string(TEST_DATA_FILE).find('.'), 4, ".idx");
	if (std::filesystem::exists(idxPath))
		std::filesystem::remove(idxPath);
}

// ========================================
// Testes
// ========================================
static void test_initialize() {
	cleanup();
	project_controller::DataManager dm;
	assert(dm.initialize(TEST_DATA_FILE) && "Falha ao inicializar");
}

static void test_create_student() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DATA_FILE);

	bool ok = dm.createStudent("Joao Silva", 1, 15051990);
	assert(ok && "Falha ao criar estudante");
	assert(dm.getActiveCount() == 1 && "Contador ativo incorreto");
}

static void test_read_by_display_id() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DATA_FILE);
	(void)dm.createStudent("Maria Santos", 2, 20101995);

	auto rec = dm.readStudent(1);
	assert(rec.has_value() && "Falha ao ler estudante");
	assert(std::strcmp(rec->name, "Maria Santos") == 0 && "Nome incorreto");
}

static void test_read_nonexistent() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DATA_FILE);

	auto rec = dm.readStudent(999);
	assert(!rec.has_value() && "Nao deveria encontrar estudante");
}

static void test_list_all() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DATA_FILE);

	(void)dm.createStudent("Aluno A", 1, 10101010);
	(void)dm.createStudent("Aluno B", 2, 20202020);
	(void)dm.createStudent("Aluno C", 3, 30303030);

	auto list = dm.listAll();
	assert(list.size() == 3 && "Deveria ter 3 estudantes");
	assert(list[0].id == 1 && "ID dinamico incorreto");
	assert(list[1].id == 2 && "ID dinamico incorreto");
	assert(list[2].id == 3 && "ID dinamico incorreto");
}

static void test_soft_delete() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DATA_FILE);
	(void)dm.createStudent("Para Deletar", 4, 11111111);
	int id = dm.getNextDisplayId() - 1;

	bool ok = dm.deleteStudent(id);
	assert(ok && "Falha ao deletar");
	assert(dm.getActiveCount() == 0 && "Deveria ter 0 ativos");
}

static void test_delete_recalculates_ids() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DATA_FILE);

	(void)dm.createStudent("Primeiro", 1, 10000001);
	(void)dm.createStudent("Segundo", 2, 10000002);
	(void)dm.createStudent("Terceiro", 3, 10000003);

	// Deleta o segundo (ID 2)
	(void)dm.deleteStudent(2);

	auto list = dm.listAll();
	assert(list.size() == 2 && "Deveria ter 2 ativos");
	assert(list[0].id == 1 && "Primeiro deveria ser ID 1");
	assert(list[1].id == 2 && "Terceiro deveria ser ID 2 (recalculado)");
}

static void test_search_by_name() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DATA_FILE);
	(void)dm.createStudent("Busca Teste", 5, 22022000);

	auto rec = dm.searchByName("Busca Teste");
	assert(rec.has_value() && "Nao encontrou por nome");
}

static void test_create_empty_name_fails() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DATA_FILE);

	bool ok = dm.createStudent("", 0, 0);
	assert(!ok && "Nao deveria permitir nome vazio");
}

static void test_get_next_display_id() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DATA_FILE);

	assert(dm.getNextDisplayId() == 1 && "Proximo ID deveria ser 1");
	(void)dm.createStudent("Teste ID", 1, 10000000);
	assert(dm.getNextDisplayId() == 2 && "Proximo ID deveria ser 2");
}

static void test_multiple_create_delete_cycle() {
	cleanup();
	project_controller::DataManager dm;
	(void)dm.initialize(TEST_DATA_FILE);

	// Cria 5
	for (int i = 0; i < 5; ++i) {
		char name[64];
		std::sprintf(name, "Estudante %d", i);
		(void)dm.createStudent(name, i, 10101000 + i);
	}
	assert(dm.getActiveCount() == 5);

	// Deleta 3
	(void)dm.deleteStudent(1);
	(void)dm.deleteStudent(2);
	(void)dm.deleteStudent(3);

	assert(dm.getActiveCount() == 2);

	auto list = dm.listAll();
	assert(list.size() == 2);
	// IDs recalculados
	assert(list[0].id == 1);
	assert(list[1].id == 2);
}

int main() {
	std::cout << "=== Testes do DataManager (Arquitetura MVC) ===\n";
	std::cout << "Executando testes...\n\n";

	runner.run("test_initialize", test_initialize);
	runner.run("test_create_student", test_create_student);
	runner.run("test_read_by_display_id", test_read_by_display_id);
	runner.run("test_read_nonexistent", test_read_nonexistent);
	runner.run("test_list_all", test_list_all);
	runner.run("test_soft_delete", test_soft_delete);
	runner.run("test_delete_recalculates_ids", test_delete_recalculates_ids);
	runner.run("test_search_by_name", test_search_by_name);
	runner.run("test_create_empty_name_fails", test_create_empty_name_fails);
	runner.run("test_get_next_display_id", test_get_next_display_id);
	runner.run("test_multiple_create_delete_cycle", test_multiple_create_delete_cycle);

	runner.summary();
	return runner.getFailed() > 0 ? 1 : 0;
}
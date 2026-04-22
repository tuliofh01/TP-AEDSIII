/**
 * @file DataManager.cpp
 * @brief Implementacao do controlador principal (CRUD).
 * @namespace project_controller
 */
#include "DataManager.hpp"
#include <algorithm>
#include <cstring>

namespace project_controller {

	// ========================================
	// Inicializacao
	// ========================================
	auto DataManager::initialize(const std::string& dataPath) -> bool {
		using namespace project_utility;
		dataPath_ = dataPath;

		// Abre arquivo de dados binario
		if (!fileMgr_.initialize(dataPath_)) {
			lastError_ = "Erro ao abrir arquivo de dados: " + dataPath_;
			return false;
		}

		// Deriva caminho do indice (.dat -> .idx)
		auto idxPath = dataPath_.substr(0, dataPath_.find_last_of('.')) + std::string(INDEX_EXT);
		if (!indexCtrl_.initialize(idxPath)) {
			lastError_ = "Erro ao inicializar indice";
			return false;
		}

		// Reconstru indice a partir dos dados existentes
		rebuildIndex();
		return true;
	}

	// ========================================
	// Create - insere novo estudante
	// ========================================
	auto DataManager::createStudent(const std::string& name, int32_t userId, uint32_t birthDate) -> bool {
		using namespace project_utility;
		if (name.empty()) {
			lastError_ = "Nome nao pode ser vazio";
			return false;
		}

		// Monta registro com ID dinamico (proxima posicao na lista ativa)
		project_model::StudentRecord rec;
		rec.setActive();
		rec.userId = userId;
		rec.birthDate = birthDate;
		rec.id = static_cast<int32_t>(fileMgr_.countActive()) + 1;
		std::strncpy(rec.name, name.c_str(), NAME_LEN - 1);
		rec.name[NAME_LEN - 1] = '\0';

		// Grava no arquivo
		auto bytes = rec.toBytes();
		if (!fileMgr_.appendRecord(bytes)) {
			lastError_ = "Erro ao escrever no arquivo";
			return false;
		}

		// Atualiza indice com offset do novo registro
		auto offset = fileMgr_.countTotal() - 1;
		(void)indexCtrl_.insert(name, rec.id, offset);
		fileMgr_.flush();
		return true;
	}

	// ========================================
	// Read - busca por ID dinamico
	// ========================================
	auto DataManager::readStudent(int32_t id) const -> std::optional<project_model::StudentRecord> {
		auto active = scanActive();
		int32_t dispId = 0;
		for (const auto& [off, rec] : active) {
			if (++dispId == id) return rec;
		}
		lastError_ = "Estudante nao encontrado com ID: " + std::to_string(id);
		return std::nullopt;
	}

	// ========================================
	// Search - busca por nome via indice hash
	// ========================================
	auto DataManager::searchByName(const std::string& name) const -> std::optional<project_model::StudentRecord> {
		auto offOpt = indexCtrl_.lookup(name);
		if (!offOpt) {
			lastError_ = "Estudante nao encontrado: " + name;
			return std::nullopt;
		}

		auto bytes = fileMgr_.readRecord(*offOpt);
		if (!bytes) {
			lastError_ = "Erro ao ler registro";
			return std::nullopt;
		}

		auto rec = project_model::StudentRecord::fromBytes(*bytes);
		if (!rec.isActive()) {
			lastError_ = "Registro deletado ou corrompido";
			return std::nullopt;
		}
		return rec;
	}

	// ========================================
	// Delete - soft delete (marca com '*')
	// ========================================
	auto DataManager::deleteStudent(int32_t id) -> bool {
		auto active = scanActive();
		int32_t dispId = 0;
		for (const auto& [off, rec] : active) {
			if (++dispId == id) {
				if (fileMgr_.markDeleted(off)) {
					(void)indexCtrl_.remove(rec.nameStr());
					fileMgr_.flush();
					return true;
				}
			}
		}
		lastError_ = "Nao foi possivel deletar estudante ID: " + std::to_string(id);
		return false;
	}

	// ========================================
	// List - todos ativos com IDs recalculados
	// ========================================
	auto DataManager::listAll() const -> std::vector<project_model::StudentRecord> {
		auto active = scanActive();
		std::vector<project_model::StudentRecord> result;
		result.reserve(active.size());
		int32_t dispId = 0;
		for (const auto& [off, rec] : active) {
			auto r = rec;
			r.id = ++dispId; // IDs dinamicos
			result.push_back(std::move(r));
		}
		return result;
	}

	// ========================================
	// Info para UI
	// ========================================
	auto DataManager::getLastError() const -> std::string { return lastError_; }

	auto DataManager::needsRebuild() const -> bool {
		if (rebuildIgnored_) return false;
		using namespace project_utility;
		auto cnt = fileMgr_.countActive();
		return cnt > 0 && (cnt % REBUILD_MODULO == 0);
	}

	void DataManager::triggerRebuild() {
		rebuildIndex();
		(void)indexCtrl_.save();
	}

	void DataManager::ignoreRebuildForSession() { rebuildIgnored_ = true; }

	auto DataManager::getNextDisplayId() const -> int32_t {
		return static_cast<int32_t>(fileMgr_.countActive()) + 1;
	}

	auto DataManager::getActiveCount() const -> int32_t {
		return static_cast<int32_t>(fileMgr_.countActive());
	}

	// ========================================
	// Metodos privados
	// ========================================
	void DataManager::rebuildIndex() {
		indexCtrl_.rebuild();
		auto active = scanActive();
		for (const auto& [off, rec] : active)
			(void)indexCtrl_.insert(rec.nameStr(), rec.id, off);
	}

	auto DataManager::scanActive() const -> std::vector<std::pair<size_t, project_model::StudentRecord>> {
		std::vector<std::pair<size_t, project_model::StudentRecord>> result;
		auto total = fileMgr_.countTotal();
		for (size_t i = 0; i < total; ++i) {
			if (!fileMgr_.isDeleted(i)) {
				if (auto bytes = fileMgr_.readRecord(i))
					result.emplace_back(i, project_model::StudentRecord::fromBytes(*bytes));
			}
		}
		return result;
	}

} // namespace project_controller
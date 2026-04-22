/**
 * @file IndexCtrl.cpp
 * @brief Implementacao do controle de indice hash.
 * @namespace project_controller
 */
#include "IndexCtrl.hpp"
#include <algorithm>
#include <fstream>

namespace project_controller {

	// ========================================
	// Inicializacao
	// ========================================
	auto IndexCtrl::initialize(const std::string& idxPath) -> bool {
		idxPath_ = idxPath;
		return load() || true; // Se nao carregar, comeca vazio
	}

	// ========================================
	// Hash DJB2
	// Algoritmo: hash = hash * 33 + char (excelente para strings curtas)
	// ========================================
	auto IndexCtrl::djb2(const std::string& name) -> uint32_t {
		uint32_t hash = 5381;
		for (auto c : name)
			hash = ((hash << 5) + hash) + static_cast<uint32_t>(c);
		return hash;
	}

	// ========================================
	// Normalizacao (lowercase)
	// ========================================
	auto IndexCtrl::normalize(const std::string& name) -> std::string {
		std::string out = name;
		std::transform(out.begin(), out.end(), out.begin(),
			[](unsigned char c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; });
		return out;
	}

	// ========================================
	// Operacoes de indice
	// ========================================
	auto IndexCtrl::insert(const std::string& name, int32_t id, size_t offset) -> bool {
		(void)id;
		map_[normalize(name)] = offset;
		return true;
	}

	auto IndexCtrl::lookup(const std::string& name) const -> std::optional<size_t> {
		auto it = map_.find(normalize(name));
		if (it != map_.end()) return it->second;
		return std::nullopt;
	}

	auto IndexCtrl::remove(const std::string& name) -> bool {
		return map_.erase(normalize(name)) > 0;
	}

	// ========================================
	// Reconstrucao
	// ========================================
	auto IndexCtrl::shouldRebuild(uint32_t activeCount) const -> bool {
		using namespace project_utility;
		return !rebuildIgnored_ && activeCount > 0 && (activeCount % REBUILD_MODULO == 0);
	}

	void IndexCtrl::rebuild() { map_.clear(); }
	void IndexCtrl::ignoreRebuild() { rebuildIgnored_ = true; }
	auto IndexCtrl::isRebuildIgnored() const -> bool { return rebuildIgnored_; }

	// ========================================
	// Persistencia (save/load)
	// ========================================
	auto IndexCtrl::save() const -> bool {
		using namespace project_utility;
		std::ofstream f(idxPath_, std::ios::binary);
		if (!f.is_open()) return false;
		f.write(reinterpret_cast<const char*>(&IDX_MAGIC), sizeof(uint32_t));
		f.write(reinterpret_cast<const char*>(&depth_), sizeof(uint32_t));
		auto sz = static_cast<uint32_t>(map_.size());
		f.write(reinterpret_cast<const char*>(&sz), sizeof(uint32_t));
		for (const auto& [key, off] : map_) {
			auto len = static_cast<uint32_t>(key.length());
			f.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
			f.write(key.data(), len);
			f.write(reinterpret_cast<const char*>(&off), sizeof(size_t));
		}
		return f.good();
	}

	auto IndexCtrl::load() -> bool {
		using namespace project_utility;
		std::ifstream f(idxPath_, std::ios::binary);
		if (!f.is_open()) return false;
		uint32_t magic;
		f.read(reinterpret_cast<char*>(&magic), sizeof(uint32_t));
		if (magic != IDX_MAGIC) return false;
		f.read(reinterpret_cast<char*>(&depth_), sizeof(uint32_t));
		uint32_t sz;
		f.read(reinterpret_cast<char*>(&sz), sizeof(uint32_t));
		map_.clear();
		for (uint32_t i = 0; i < sz; ++i) {
			uint32_t len;
			f.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
			std::string key(len, ' ');
			f.read(key.data(), len);
			size_t off;
			f.read(reinterpret_cast<char*>(&off), sizeof(size_t));
			map_[key] = off;
		}
		return f.good();
	}

	void IndexCtrl::clear() {
		using namespace project_utility;
		map_.clear();
		depth_ = IDX_INITIAL_DEPTH;
		rebuildIgnored_ = false;
	}

} // namespace project_controller
#include "IndexCtrl.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace project_controller {

	auto IndexCtrl::initialize(const std::string& idxPath) -> bool {
		idxPath_ = idxPath;
		return load() || true;
	}

	auto IndexCtrl::djb2(const std::string& str) -> uint32_t {
		uint32_t hash = 5381;
		for (auto c : str)
			hash = ((hash << 5) + hash) + static_cast<uint32_t>(c);
		return hash;
	}

	auto IndexCtrl::normalize(const std::string& str) -> std::string {
		std::string out = str;
		std::transform(out.begin(), out.end(), out.begin(),
			[](unsigned char c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; });
		return out;
	}

	auto IndexCtrl::insert(const std::string& key, uint32_t chunkIndex, uint32_t recordIndex) -> bool {
		IndexEntry entry{chunkIndex, recordIndex};
		map_[normalize(key)] = entry;
		return true;
	}

	auto IndexCtrl::lookup(const std::string& key) const -> std::optional<IndexValue> {
		auto it = map_.find(normalize(key));
		if (it != map_.end()) {
			return IndexValue{it->second.chunkIndex, it->second.recordIndex};
		}
		return std::nullopt;
	}

	auto IndexCtrl::remove(const std::string& key) -> bool {
		return map_.erase(normalize(key)) > 0;
	}

	auto IndexCtrl::nextId(const std::string& prefix) const -> uint32_t {
		uint32_t maxId = 0;
		auto normPrefix = normalize(prefix);
		for (const auto& [key, _] : map_) {
			if (key.substr(0, normPrefix.size()) == normPrefix) {
				auto numStr = key.substr(normPrefix.size());
				try {
					uint32_t val = static_cast<uint32_t>(std::stoul(numStr));
					if (val > maxId) maxId = val;
				} catch (...) {}
			}
		}
		return maxId + 1;
	}

	void IndexCtrl::rebuild() {
		map_.clear();
	}

	auto IndexCtrl::save() const -> bool {
		using namespace project_utility;
		std::ofstream f(idxPath_, std::ios::binary);
		if (!f.is_open()) return false;
		f.write(reinterpret_cast<const char*>(&IDX_MAGIC), sizeof(uint32_t));
		f.write(reinterpret_cast<const char*>(&depth_), sizeof(uint32_t));
		auto sz = static_cast<uint32_t>(map_.size());
		f.write(reinterpret_cast<const char*>(&sz), sizeof(uint32_t));
		for (const auto& [key, entry] : map_) {
			auto len = static_cast<uint32_t>(key.length());
			f.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
			f.write(key.data(), len);
			f.write(reinterpret_cast<const char*>(&entry.chunkIndex), sizeof(uint32_t));
			f.write(reinterpret_cast<const char*>(&entry.recordIndex), sizeof(uint32_t));
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
			IndexEntry entry;
			f.read(reinterpret_cast<char*>(&entry.chunkIndex), sizeof(uint32_t));
			f.read(reinterpret_cast<char*>(&entry.recordIndex), sizeof(uint32_t));
			map_[key] = entry;
		}
		return f.good();
	}

	void IndexCtrl::clear() {
		using namespace project_utility;
		map_.clear();
		depth_ = IDX_INITIAL_DEPTH;
	}

} // namespace project_controller

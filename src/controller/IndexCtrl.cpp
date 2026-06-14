#include "IndexCtrl.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace project_controller {

	// Sets the index file path and attempts to load existing data.
	auto IndexCtrl::initialize(const std::string& idxPath) -> bool {
		idxPath_ = idxPath;
		return load() || true;
	}

	// DJB2 hash function for string keys.
	auto IndexCtrl::djb2(const std::string& str) -> uint32_t {
		uint32_t hash = 5381;
		for (auto c : str)
			hash = ((hash << 5) + hash) + static_cast<uint32_t>(c);
		return hash;
	}

	// Converts string to lowercase for case-insensitive key matching.
	auto IndexCtrl::normalize(const std::string& str) -> std::string {
		auto out = str;
		std::transform(out.begin(), out.end(), out.begin(),
			[](unsigned char c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; });
		return out;
	}

	// Inserts or updates a key-index pair into the map.
	auto IndexCtrl::insert(const std::string& key, uint32_t chunkIndex, uint32_t recordIndex) -> bool {
		IndexEntry entry{chunkIndex, recordIndex};
		map_[normalize(key)] = entry;
		return true;
	}

	// Looks up a key and returns its chunk/record indices if found.
	auto IndexCtrl::lookup(const std::string& key) const -> std::optional<IndexValue> {
		auto it = map_.find(normalize(key));
		if (it != map_.end()) {
			return IndexValue{it->second.chunkIndex, it->second.recordIndex};
		}
		return std::nullopt;
	}

	// Removes a key from the index. Returns true if the key existed.
	auto IndexCtrl::remove(const std::string& key) -> bool {
		return map_.erase(normalize(key)) > 0;
	}

	// Returns the next available ID by finding the max existing ID for the given prefix.
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

	// Clears the map — next save will write an empty index.
	void IndexCtrl::rebuild() {
		map_.clear();
	}

	// Serializes the index map to a binary file.
	auto IndexCtrl::save() const -> bool {
		using namespace project_utility;
		std::ofstream outputFile(idxPath_, std::ios::binary);
		if (!outputFile.is_open()) return false;
		outputFile.write(reinterpret_cast<const char*>(&IDX_MAGIC), sizeof(uint32_t));
		outputFile.write(reinterpret_cast<const char*>(&depth_), sizeof(uint32_t));
		auto mapSize = static_cast<uint32_t>(map_.size());
		outputFile.write(reinterpret_cast<const char*>(&mapSize), sizeof(uint32_t));
		for (const auto& [key, entry] : map_) {
			auto keyLength = static_cast<uint32_t>(key.length());
			outputFile.write(reinterpret_cast<const char*>(&keyLength), sizeof(uint32_t));
			outputFile.write(key.data(), keyLength);
			outputFile.write(reinterpret_cast<const char*>(&entry.chunkIndex), sizeof(uint32_t));
			outputFile.write(reinterpret_cast<const char*>(&entry.recordIndex), sizeof(uint32_t));
		}
		return outputFile.good();
	}

	// Deserializes the index map from a binary file.
	auto IndexCtrl::load() -> bool {
		using namespace project_utility;
		std::ifstream inputFile(idxPath_, std::ios::binary);
		if (!inputFile.is_open()) return false;
		uint32_t magic;
		inputFile.read(reinterpret_cast<char*>(&magic), sizeof(uint32_t));
		if (magic != IDX_MAGIC) return false;
		inputFile.read(reinterpret_cast<char*>(&depth_), sizeof(uint32_t));
		uint32_t mapSize;
		inputFile.read(reinterpret_cast<char*>(&mapSize), sizeof(uint32_t));
		map_.clear();
		for (uint32_t entryIndex = 0; entryIndex < mapSize; ++entryIndex) {
			uint32_t keyLength;
			inputFile.read(reinterpret_cast<char*>(&keyLength), sizeof(uint32_t));
			std::string key(keyLength, ' ');
			inputFile.read(key.data(), keyLength);
			IndexEntry entry;
			inputFile.read(reinterpret_cast<char*>(&entry.chunkIndex), sizeof(uint32_t));
			inputFile.read(reinterpret_cast<char*>(&entry.recordIndex), sizeof(uint32_t));
			map_[key] = entry;
		}
		return inputFile.good();
	}

	// Resets the index to its initial empty state.
	void IndexCtrl::clear() {
		using namespace project_utility;
		map_.clear();
		depth_ = IDX_INITIAL_DEPTH;
	}

} // namespace project_controller

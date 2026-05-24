#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

#include "../utility/Constants.hpp"
#include "../utility/Enums.hpp"

namespace project_controller {

	struct IndexValue {
		uint32_t chunkIndex = 0;
		uint32_t recordIndex = 0;
	};

	class IndexCtrl {
	public:
		IndexCtrl() = default;
		~IndexCtrl() = default;

		IndexCtrl(const IndexCtrl&) = delete;
		IndexCtrl& operator=(const IndexCtrl&) = delete;
		IndexCtrl(IndexCtrl&&) noexcept = default;
		IndexCtrl& operator=(IndexCtrl&&) noexcept = default;

		[[nodiscard]] bool initialize(const std::string& idxPath);
		[[nodiscard]] bool save() const;
		[[nodiscard]] bool load();

		void clear();
		void rebuild();

		bool insert(const std::string& key, uint32_t chunkIndex, uint32_t recordIndex);
		std::optional<IndexValue> lookup(const std::string& key) const;
		bool remove(const std::string& key);

		[[nodiscard]] uint32_t nextId(const std::string& prefix) const;

		[[nodiscard]] size_t size() const { return map_.size(); }

	private:
		struct IndexEntry {
			uint32_t chunkIndex;
			uint32_t recordIndex;
		};

		static uint32_t djb2(const std::string& str);
		static std::string normalize(const std::string& str);

		std::unordered_map<std::string, IndexEntry> map_;
		std::string idxPath_;
		uint32_t depth_ = project_utility::IDX_INITIAL_DEPTH;
	};

} // namespace project_controller

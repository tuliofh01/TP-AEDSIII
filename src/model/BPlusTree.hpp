// B+ Tree implementation for on-disk indexed storage.
// Supports insert, search (exact + prefix range), and erase operations.
// Pages are fixed-size; header and nodes are persisted in a binary file chunk.

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <utility>

#include "Record.hpp"

namespace project_model {

	struct BTreeFileHeader {
		int64_t rootPageId = -1;
		size_t entryCount = 0;
		size_t pageCount = 0;

		[[nodiscard]] std::vector<std::byte> toBytes() const;
		static BTreeFileHeader fromBytes(const std::vector<std::byte>& data);
	};

	class BPlusTree {
	public:
		static constexpr size_t PAGE_SIZE = project_utility::BTREE_PAGE_SIZE;
		static constexpr size_t HEADER_SIZE = project_utility::BTREE_HEADER_SIZE;
		static constexpr size_t KEY_SIZE = project_utility::BTREE_KEY_SIZE;
		static constexpr size_t CHUNK_HEADER = sizeof(int64_t) + sizeof(size_t) + sizeof(size_t);
		static constexpr size_t INTERNAL_ENTRY = project_utility::BTREE_INTERNAL_ENTRY_SIZE;
		static constexpr size_t INTERNAL_MAX = project_utility::BTREE_INTERNAL_MAX_KEYS;
		static constexpr size_t INTERNAL_MIN = project_utility::BTREE_INTERNAL_MIN_KEYS;

		// Leaf constants computed from actual BTreeLeafValue size
		static constexpr size_t LEAF_VALUE_SIZE = sizeof(BTreeLeafValue);
		static constexpr size_t LEAF_ENTRY = KEY_SIZE + LEAF_VALUE_SIZE;
		static constexpr size_t LEAF_MAX = (PAGE_SIZE - HEADER_SIZE) / LEAF_ENTRY;
		static constexpr size_t LEAF_MIN = LEAF_MAX / 2;

		BPlusTree() = default;
		~BPlusTree() = default;

		BPlusTree(const BPlusTree&) = delete;
		BPlusTree& operator=(const BPlusTree&) = delete;

		// Lifecycle
		bool initialize(std::fstream& file, size_t chunkOffset, size_t initialPages);

		// CRUD operations
		bool insert(const std::string& key, const BTreeLeafValue& value);
		std::optional<BTreeLeafValue> search(const std::string& key) const;
		std::vector<std::pair<std::string, BTreeLeafValue>>
			searchRange(const std::string& prefix) const;
		bool erase(const std::string& key);

		size_t size() const { return fileHeader_.entryCount; }
		int64_t rootId() const { return fileHeader_.rootPageId; }
		bool empty() const { return fileHeader_.pageCount == 0; }

	private:
		std::fstream* file_ = nullptr;
		size_t chunkOffset_ = 0;
		BTreeFileHeader fileHeader_;

		static std::string formatKey(const std::string& raw);
		static std::string extractKey(const std::byte* buf);

		int64_t allocatePage(char type);
		void readPage(int64_t pageId, std::vector<std::byte>& buf) const;
		void writePage(int64_t pageId, const std::vector<std::byte>& buf);
		std::vector<std::byte> newPage(char type);

		void saveHeader();
		void loadHeader();

		// Page field accessors
		static char& pageType(std::byte* pageBuf);
		static uint16_t& pageNumKeys(std::byte* pageBuf);
		static int64_t& pageParent(std::byte* pageBuf);
		static int64_t& pageNextLeaf(std::byte* pageBuf);
		static int64_t& pagePrevLeaf(std::byte* pageBuf);

		// Internal node accessors
		static int64_t& internalChild(std::byte* pageBuf, size_t index);
		static std::byte* internalKeyPtr(std::byte* pageBuf, size_t index);

		// Leaf node accessors
		static std::byte* leafEntryPtr(std::byte* pageBuf, size_t index);
		static void leafSetEntry(std::byte* pageBuf, size_t index,
			const std::string& key, const BTreeLeafValue& value);
		static void leafGetEntry(const std::byte* pageBuf, size_t index,
			std::string& key, BTreeLeafValue& value);

		// Core algorithms
		void insertNonFull(int64_t nodeId, const std::string& key,
			const BTreeLeafValue& value);
		void splitChild(int64_t parentId, size_t childIdx, int64_t childId);

		std::optional<BTreeLeafValue> searchInNode(int64_t nodeId,
			const std::string& key) const;

		void collectRange(int64_t nodeId, const std::string& prefix,
			std::vector<std::pair<std::string, BTreeLeafValue>>& out) const;

		bool eraseFromNode(int64_t nodeId, const std::string& key);
		void borrowOrMerge(int64_t parentId, size_t idx);
	};

} // namespace project_model

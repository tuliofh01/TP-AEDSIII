#include "BPlusTree.hpp"
#include <cstring>
#include <algorithm>

namespace project_model {

	// ========================================
	// BTreeFileHeader
	// ========================================
	auto BTreeFileHeader::toBytes() const -> std::vector<std::byte> {
		return serializeRecord(*this);
	}

	auto BTreeFileHeader::fromBytes(const std::vector<std::byte>& data) -> BTreeFileHeader {
		return deserializeRecord<BTreeFileHeader>(data);
	}

	// ========================================
	// BPlusTree implementation
	// ========================================

	bool BPlusTree::initialize(std::fstream& file, size_t chunkOffset, size_t initialPages) {
		file_ = &file;
		chunkOffset_ = chunkOffset;

		// Try to read existing header
		loadHeader();

		// If no pages, allocate initial pages
		if (fileHeader_.pageCount == 0) {
			fileHeader_.pageCount = initialPages;
			for (size_t i = 0; i < initialPages; ++i) {
				auto page = newPage('L');
				writePage(static_cast<int64_t>(i), page);
			}
			fileHeader_.rootPageId = 0;
			saveHeader();
		}

		return true;
	}

	std::string BPlusTree::formatKey(const std::string& raw) {
		if (raw.size() >= KEY_SIZE) return raw.substr(0, KEY_SIZE - 1);
		return raw;
	}

	std::string BPlusTree::extractKey(const std::byte* pageBuf) {
		return std::string(reinterpret_cast<const char*>(pageBuf), KEY_SIZE).c_str();
	}

	int64_t BPlusTree::allocatePage(char type) {
		int64_t pageId = static_cast<int64_t>(fileHeader_.pageCount);
		auto page = newPage(type);
		writePage(pageId, page);
		fileHeader_.pageCount++;
		saveHeader();
		return pageId;
	}

	std::vector<std::byte> BPlusTree::newPage(char type) {
		std::vector<std::byte> page(PAGE_SIZE, std::byte{0});
		pageType(page.data()) = type;
		pageParent(page.data()) = -1;
		pageNextLeaf(page.data()) = -1;
		pagePrevLeaf(page.data()) = -1;
		return page;
	}

	void BPlusTree::readPage(int64_t pageId, std::vector<std::byte>& pageBuf) const {
		pageBuf.resize(PAGE_SIZE);
		size_t offset = chunkOffset_ + CHUNK_HEADER + static_cast<size_t>(pageId) * PAGE_SIZE;
		file_->seekg(static_cast<std::streamoff>(offset), std::ios::beg);
		file_->read(reinterpret_cast<char*>(pageBuf.data()), PAGE_SIZE);
	}

	void BPlusTree::writePage(int64_t pageId, const std::vector<std::byte>& pageBuf) {
		size_t offset = chunkOffset_ + CHUNK_HEADER + static_cast<size_t>(pageId) * PAGE_SIZE;
		file_->seekp(static_cast<std::streamoff>(offset), std::ios::beg);
		file_->write(reinterpret_cast<const char*>(pageBuf.data()), PAGE_SIZE);
		file_->flush();
	}

	void BPlusTree::saveHeader() {
		auto hdrBytes = fileHeader_.toBytes();
		file_->seekp(static_cast<std::streamoff>(chunkOffset_), std::ios::beg);
		file_->write(reinterpret_cast<const char*>(hdrBytes.data()), hdrBytes.size());
	}

	void BPlusTree::loadHeader() {
		std::vector<std::byte> hdrBytes(CHUNK_HEADER);
		file_->seekg(static_cast<std::streamoff>(chunkOffset_), std::ios::beg);
		file_->read(reinterpret_cast<char*>(hdrBytes.data()), CHUNK_HEADER);
		fileHeader_ = BTreeFileHeader::fromBytes(hdrBytes);
	}

	// ========================================
	// Page field accessors
	// ========================================
	// Page header layout:
	// [0] char    pageType     (1 byte)
	// [1] uint16  pageNumKeys  (2 bytes)
	// [3] int64   parentId     (8 bytes)
	// [11] int64  nextLeafId   (8 bytes)
	// [19] int64  prevLeafId   (8 bytes)
	// Total header = 27 bytes; HEADER_SIZE=31 (4 bytes padding for alignment)
	static constexpr auto  PAGE_TYPE_OFF    = size_t{0};
	static constexpr auto  NUM_KEYS_OFF     = size_t{1};
	static constexpr auto  PARENT_OFF       = size_t{3};
	static constexpr auto  NEXT_LEAF_OFF    = size_t{11};
	static constexpr auto  PREV_LEAF_OFF    = size_t{19};

	char& BPlusTree::pageType(std::byte* pageBuf) {
		return *reinterpret_cast<char*>(pageBuf + PAGE_TYPE_OFF);
	}

	uint16_t& BPlusTree::pageNumKeys(std::byte* pageBuf) {
		return *reinterpret_cast<uint16_t*>(pageBuf + NUM_KEYS_OFF);
	}

	int64_t& BPlusTree::pageParent(std::byte* pageBuf) {
		return *reinterpret_cast<int64_t*>(pageBuf + PARENT_OFF);
	}

	int64_t& BPlusTree::pageNextLeaf(std::byte* pageBuf) {
		return *reinterpret_cast<int64_t*>(pageBuf + NEXT_LEAF_OFF);
	}

	int64_t& BPlusTree::pagePrevLeaf(std::byte* pageBuf) {
		return *reinterpret_cast<int64_t*>(pageBuf + PREV_LEAF_OFF);
	}

	// ========================================
	// Internal node accessors
	// ========================================
	int64_t& BPlusTree::internalChild(std::byte* pageBuf, size_t index) {
		return *reinterpret_cast<int64_t*>(pageBuf + HEADER_SIZE + index * INTERNAL_ENTRY);
	}

	std::byte* BPlusTree::internalKeyPtr(std::byte* pageBuf, size_t index) {
		return pageBuf + HEADER_SIZE + INTERNAL_ENTRY + (index - 1) * INTERNAL_ENTRY;
	}

	// ========================================
	// Leaf node accessors
	// ========================================
	std::byte* BPlusTree::leafEntryPtr(std::byte* pageBuf, size_t index) {
		return pageBuf + HEADER_SIZE + index * LEAF_ENTRY;
	}

	void BPlusTree::leafSetEntry(std::byte* pageBuf, size_t index,
		const std::string& key, const BTreeLeafValue& value) {
		auto* entry = leafEntryPtr(pageBuf, index);
		std::memset(entry, 0, LEAF_ENTRY);
		size_t copyLen = std::min(key.size(), KEY_SIZE - 1);
		std::memcpy(entry, key.data(), copyLen);
		auto valBytes = serializeRecord(value);
		std::memcpy(entry + KEY_SIZE, valBytes.data(), LEAF_VALUE_SIZE);
	}

	void BPlusTree::leafGetEntry(const std::byte* pageBuf, size_t index,
		std::string& key, BTreeLeafValue& value) {
		auto* entry = pageBuf + HEADER_SIZE + index * LEAF_ENTRY;
		key = std::string(reinterpret_cast<const char*>(entry), KEY_SIZE).c_str();
		std::vector<std::byte> valBytes(entry + KEY_SIZE, entry + KEY_SIZE + LEAF_VALUE_SIZE);
		value = deserializeRecord<BTreeLeafValue>(valBytes);
	}

	// ========================================
	// Search
	// ========================================
	std::optional<BTreeLeafValue> BPlusTree::search(const std::string& key) const {
		if (empty()) return std::nullopt;
		return searchInNode(fileHeader_.rootPageId, formatKey(key));
	}

	std::optional<BTreeLeafValue> BPlusTree::searchInNode(int64_t nodeId,
		const std::string& key) const {
		std::vector<std::byte> page(PAGE_SIZE);
		readPage(nodeId, page);
		auto* pageBuf = page.data();

		if (pageType(pageBuf) == 'L') {
			// Leaf node: scan entries
			uint16_t keyCount = pageNumKeys(pageBuf);
			for (uint16_t i = 0; i < keyCount; ++i) {
				std::string entryKey;
				BTreeLeafValue val;
				leafGetEntry(pageBuf, i, entryKey, val);
				if (entryKey == key) return val;
			}
			return std::nullopt;
		}

		// Internal node: find correct child
		uint16_t keyCount = pageNumKeys(pageBuf);
		size_t childIdx = 0;
		for (uint16_t i = 1; i <= keyCount; ++i) {
			std::string sepKey;
			BTreeLeafValue dummy;
			auto* keyPtr = internalKeyPtr(pageBuf, i);
			sepKey = extractKey(keyPtr);
			if (key < sepKey) {
				childIdx = static_cast<size_t>(i - 1);
				goto descend;
			}
			childIdx = static_cast<size_t>(i);
		}
		descend:
		int64_t childId = internalChild(pageBuf, childIdx);
		return searchInNode(childId, key);
	}

	// ========================================
	// Range scan
	// ========================================
	std::vector<std::pair<std::string, BTreeLeafValue>>
	BPlusTree::searchRange(const std::string& prefix) const {
		std::vector<std::pair<std::string, BTreeLeafValue>> result;
		if (empty()) return result;
		collectRange(fileHeader_.rootPageId, formatKey(prefix), result);
		return result;
	}

	void BPlusTree::collectRange(int64_t nodeId, const std::string& prefix,
		std::vector<std::pair<std::string, BTreeLeafValue>>& out) const {

		std::vector<std::byte> page(PAGE_SIZE);
		readPage(nodeId, page);
		auto* pageBuf = page.data();

		if (pageType(pageBuf) == 'I') {
			uint16_t keyCount = pageNumKeys(pageBuf);
			// Find first child that might have matches
			collectRange(internalChild(pageBuf, 0), prefix, out);
			for (uint16_t i = 1; i <= keyCount; ++i) {
				collectRange(internalChild(pageBuf, i), prefix, out);
			}
			return;
		}

		// Leaf: start from first matching entry, walk right
		int64_t currentId = nodeId;
		bool started = false;

		while (currentId >= 0) {
			std::vector<std::byte> leafPage(PAGE_SIZE);
			readPage(currentId, leafPage);
			auto* leftPageBuf = leafPage.data();
			uint16_t keyCount = pageNumKeys(leftPageBuf);

			for (uint16_t i = 0; i < keyCount; ++i) {
				std::string key;
				BTreeLeafValue val;
				leafGetEntry(leftPageBuf, i, key, val);

				if (!started) {
					if (key.substr(0, prefix.size()) < prefix) continue;
					if (key.substr(0, prefix.size()) > prefix) return;
					started = true;
				}

				if (key.substr(0, prefix.size()) != prefix) return;
				out.emplace_back(key, val);
			}

			currentId = pageNextLeaf(leftPageBuf);
		}
	}

	// ========================================
	// Insert
	// ========================================
	bool BPlusTree::insert(const std::string& key, const BTreeLeafValue& value) {
		auto fkey = formatKey(key);
		if (empty()) {
			// Initialize root leaf
			auto page = newPage('L');
			leafSetEntry(page.data(), 0, fkey, value);
			pageNumKeys(page.data()) = 1;
			writePage(fileHeader_.rootPageId, page);
			fileHeader_.pageCount = 1;
			fileHeader_.entryCount = 1;
			saveHeader();
			return true;
		}

		// Check root for overflow
		std::vector<std::byte> rootPage(PAGE_SIZE);
		readPage(fileHeader_.rootPageId, rootPage);
		auto* rootBuf = rootPage.data();

		if (pageType(rootBuf) == 'L' && pageNumKeys(rootBuf) >= LEAF_MAX) {
			// Split root
			int64_t newRootId = allocatePage('I');
			int64_t newLeafId = allocatePage('L');

			// Move half of root's entries to new leaf
			auto oldRootId = fileHeader_.rootPageId;
			auto* oldBuf = rootPage.data();
			uint16_t half = LEAF_MAX / 2;

			std::vector<std::byte> newLeaf(PAGE_SIZE);
			std::memset(newLeaf.data(), 0, PAGE_SIZE);
			pageType(newLeaf.data()) = 'L';
			pagePrevLeaf(newLeaf.data()) = oldRootId;
			pageNextLeaf(newLeaf.data()) = pageNextLeaf(oldBuf);
			pageNextLeaf(oldBuf) = newLeafId;
			pageType(oldBuf) = 'L';

			// Copy entries to new leaf
			uint16_t moved = 0;
			BTreeLeafValue firstVal;
			std::string firstKey;
			for (uint16_t i = half; i < pageNumKeys(oldBuf); ++i) {
				leafGetEntry(oldBuf, i, firstKey, firstVal);
				leafSetEntry(newLeaf.data(), moved, firstKey, firstVal);
				moved++;
			}
			pageNumKeys(newLeaf.data()) = moved;
			pageNumKeys(oldBuf) = half - 1;
			// Actually half entries stay, half go to new leaf
			// Let me recalculate: old has N entries. keep first N/2, move rest

			// Hmm, let me restart this logic properly
			// Actually, let me just rebuild from scratch for the root split

			// For now: old leaf becomes first half, new leaf gets second half
			pageNumKeys(oldBuf) = half;
			pageNumKeys(newLeaf.data()) = LEAF_MAX - half;

			// Get split key (first key of new leaf)
			std::string splitKey;
			leafGetEntry(newLeaf.data(), 0, splitKey, firstVal);

			// Set up new root (internal node)
			pageType(rootBuf) = 'I';
			pageNumKeys(rootBuf) = 0;
			pageParent(rootBuf) = -1;
			pageNextLeaf(rootBuf) = -1;
			pagePrevLeaf(rootBuf) = -1;
			internalChild(rootBuf, 0) = oldRootId;
			pageParent(oldBuf) = newRootId;
			pageParent(newLeaf.data()) = newRootId;

			// Insert split key and new child pointer
			pageNumKeys(rootBuf) = 1;
			std::memcpy(internalKeyPtr(rootBuf, 1), splitKey.data(),
				std::min(splitKey.size(), KEY_SIZE - 1));
			internalChild(rootBuf, 1) = newLeafId;

			writePage(oldRootId, rootPage);
			writePage(newLeafId, newLeaf);
			fileHeader_.rootPageId = newRootId;
			saveHeader();
		}

		insertNonFull(fileHeader_.rootPageId, fkey, value);
		fileHeader_.entryCount++;
		saveHeader();
		return true;
	}

	void BPlusTree::insertNonFull(int64_t nodeId, const std::string& key,
		const BTreeLeafValue& value) {
		std::vector<std::byte> page(PAGE_SIZE);
		readPage(nodeId, page);
		auto* pageBuf = page.data();

		if (pageType(pageBuf) == 'L') {
			// Leaf: insert in sorted position
			uint16_t keyCount = pageNumKeys(pageBuf);
			int16_t pos = static_cast<int16_t>(keyCount) - 1;

			while (pos >= 0) {
				std::string existingKey;
				BTreeLeafValue dummyVal;
				leafGetEntry(pageBuf, static_cast<size_t>(pos), existingKey, dummyVal);
				if (key >= existingKey) break;
				pos--;
			}
			pos++; // insert at pos+1

			// Shift entries right
			for (uint16_t i = keyCount; i > static_cast<uint16_t>(pos); --i) {
				std::string srcKey;
				BTreeLeafValue srcVal;
				leafGetEntry(pageBuf, i - 1, srcKey, srcVal);
				leafSetEntry(pageBuf, i, srcKey, srcVal);
			}

			leafSetEntry(pageBuf, static_cast<size_t>(pos), key, value);
			pageNumKeys(pageBuf) = keyCount + 1;
			writePage(nodeId, page);
			return;
		}

		// Internal: find child to descend into
		uint16_t keyCount = pageNumKeys(pageBuf);
		size_t childIdx = 0;
		for (uint16_t i = 1; i <= keyCount; ++i) {
			auto* keyPtr = internalKeyPtr(pageBuf, i);
			auto sepKey = extractKey(keyPtr);
			if (key < sepKey) {
				childIdx = static_cast<size_t>(i - 1);
				goto descend_internal;
			}
			childIdx = static_cast<size_t>(i);
		}
		descend_internal:

		int64_t childId = internalChild(pageBuf, childIdx);

		// Check if child is full
		std::vector<std::byte> childPage(PAGE_SIZE);
		readPage(childId, childPage);
		auto* childBuf = childPage.data();
		bool childFull = (pageType(childBuf) == 'L' && pageNumKeys(childBuf) >= LEAF_MAX)
			|| (pageType(childBuf) == 'I' && pageNumKeys(childBuf) >= INTERNAL_MAX);

		if (childFull) {
			splitChild(nodeId, childIdx, childId);
			// After split, determine which child to descend into
			readPage(nodeId, page);
			pageBuf = page.data();
			keyCount = pageNumKeys(pageBuf);
			// Re-read the children
			for (uint16_t i = 1; i <= keyCount; ++i) {
				auto* keyPtr = internalKeyPtr(pageBuf, i);
				auto sepKey = extractKey(keyPtr);
				if (key < sepKey) {
					childId = internalChild(pageBuf, static_cast<size_t>(i - 1));
					goto descend_after_split;
				}
			}
			childId = internalChild(pageBuf, static_cast<size_t>(keyCount));
		}
		descend_after_split:

		insertNonFull(childId, key, value);
	}

	void BPlusTree::splitChild(int64_t parentId, size_t childIdx, int64_t childId) {
		std::vector<std::byte> parentPage(PAGE_SIZE);
		readPage(parentId, parentPage);
		auto* parentBuf = parentPage.data();

		std::vector<std::byte> childPage(PAGE_SIZE);
		readPage(childId, childPage);
		auto* childBuf = childPage.data();

		bool isLeaf = (pageType(childBuf) == 'L');
		int64_t newId = allocatePage(isLeaf ? 'L' : 'I');
		std::vector<std::byte> newPage_pageBuf(PAGE_SIZE);
		std::memset(newPage_pageBuf.data(), 0, PAGE_SIZE);
		auto* newBuf = newPage_pageBuf.data();

		pageParent(newBuf) = parentId;
		pageType(newBuf) = pageType(childBuf);

		uint16_t keyCount = pageNumKeys(childBuf);
		uint16_t half = isLeaf ? LEAF_MAX / 2 : INTERNAL_MAX / 2;
		uint16_t moved = keyCount - half;

		if (isLeaf) {
			// Copy right half to new leaf
			for (uint16_t i = half; i < keyCount; ++i) {
				std::string entryKey;
				BTreeLeafValue entryValue;
				leafGetEntry(childBuf, i, entryKey, entryValue);
				leafSetEntry(newBuf, i - half, entryKey, entryValue);
			}
			pageNumKeys(newBuf) = moved;
			pageNumKeys(childBuf) = half;

			// Update leaf chain
			pageNextLeaf(newBuf) = pageNextLeaf(childBuf);
			pagePrevLeaf(newBuf) = childId;
			pageNextLeaf(childBuf) = newId;

			// Get split key
			std::string splitKey;
			BTreeLeafValue dummyVal;
			leafGetEntry(newBuf, 0, splitKey, dummyVal);

			// Insert into parent
			uint16_t pn = pageNumKeys(parentBuf);
			for (uint16_t i = pn; i > static_cast<uint16_t>(childIdx + 1); --i) {
				internalChild(parentBuf, i) = internalChild(parentBuf, i - 1);
				std::memcpy(internalKeyPtr(parentBuf, i), internalKeyPtr(parentBuf, i - 1), KEY_SIZE);
			}

			pageNumKeys(parentBuf) = pn + 1;
			internalChild(parentBuf, childIdx + 1) = newId;
			std::memcpy(internalKeyPtr(parentBuf, childIdx + 1), splitKey.data(),
				std::min(splitKey.size(), KEY_SIZE - 1));
		} else {
			// Internal node: move separator keys and children
			// Copy child pointer
			internalChild(newBuf, 0) = internalChild(childBuf, half + 1);
			// Copy keys and their children
			for (uint16_t i = half + 1; i <= keyCount; ++i) {
				uint16_t newIdx = i - half - 1;
				if (newIdx > 0)
					std::memcpy(internalKeyPtr(newBuf, newIdx), internalKeyPtr(childBuf, i), KEY_SIZE);
				internalChild(newBuf, newIdx + 1) = internalChild(childBuf, i + 1);
			}
			pageNumKeys(newBuf) = keyCount - half - 1;
			pageNumKeys(childBuf) = half;

			// The middle key goes up to parent
			auto splitKey = extractKey(internalKeyPtr(childBuf, half + 1));

			uint16_t pn = pageNumKeys(parentBuf);
			for (uint16_t i = pn; i > static_cast<uint16_t>(childIdx + 1); --i) {
				internalChild(parentBuf, i) = internalChild(parentBuf, i - 1);
				std::memcpy(internalKeyPtr(parentBuf, i), internalKeyPtr(parentBuf, i - 1), KEY_SIZE);
			}
			pageNumKeys(parentBuf) = pn + 1;
			internalChild(parentBuf, childIdx + 1) = newId;
			std::memcpy(internalKeyPtr(parentBuf, childIdx + 1), splitKey.data(),
				std::min(splitKey.size(), KEY_SIZE - 1));
		}

		writePage(childId, childPage);
		writePage(newId, newPage_pageBuf);
		writePage(parentId, parentPage);
	}

	// ========================================
	// Erase
	// ========================================
	bool BPlusTree::erase(const std::string& key) {
		if (empty()) return false;
		auto fkey = formatKey(key);
		if (eraseFromNode(fileHeader_.rootPageId, fkey)) {
			fileHeader_.entryCount--;

			// If root has 0 keys, make its only child the new root
			std::vector<std::byte> rootPage(PAGE_SIZE);
			readPage(fileHeader_.rootPageId, rootPage);
			auto* pageBuf = rootPage.data();
			if (pageType(pageBuf) == 'I' && pageNumKeys(pageBuf) == 0 && fileHeader_.pageCount > 1) {
				fileHeader_.rootPageId = internalChild(pageBuf, 0);
				pageParent(pageBuf) = -1;
			}

			saveHeader();
			return true;
		}
		return false;
	}

	bool BPlusTree::eraseFromNode(int64_t nodeId, const std::string& key) {
		std::vector<std::byte> page(PAGE_SIZE);
		readPage(nodeId, page);
		auto* pageBuf = page.data();

		if (pageType(pageBuf) == 'L') {
			uint16_t keyCount = pageNumKeys(pageBuf);
			for (uint16_t i = 0; i < keyCount; ++i) {
				std::string entryKey;
				BTreeLeafValue entryValue;
				leafGetEntry(pageBuf, i, entryKey, entryValue);
				if (entryKey == key) {
					// Shift left
					for (uint16_t j = i; j + 1 < keyCount; ++j) {
						std::string sjKey;
						BTreeLeafValue sjVal;
						leafGetEntry(pageBuf, j + 1, sjKey, sjVal);
						leafSetEntry(pageBuf, j, sjKey, sjVal);
					}
					pageNumKeys(pageBuf) = keyCount - 1;
					writePage(nodeId, page);
					return true;
				}
			}
			return false;
		}

		// Internal node
		uint16_t keyCount = pageNumKeys(pageBuf);
		size_t childIdx = 0;
		for (uint16_t i = 1; i <= keyCount; ++i) {
			auto* keyPtr = internalKeyPtr(pageBuf, i);
			auto sepKey = extractKey(keyPtr);
			if (key < sepKey) {
				childIdx = static_cast<size_t>(i - 1);
				goto erase_descend;
			}
			childIdx = static_cast<size_t>(i);
		}
		erase_descend:

		int64_t childId = internalChild(pageBuf, childIdx);
		bool removed = eraseFromNode(childId, key);

		if (removed) {
			// Check if child is underfull
			std::vector<std::byte> childPage(PAGE_SIZE);
			readPage(childId, childPage);
			auto* childPageBuf = childPage.data();
			size_t minKeys = (pageType(childPageBuf) == 'L') ? LEAF_MIN : INTERNAL_MIN;
			if (pageNumKeys(childPageBuf) < minKeys && nodeId != childId) {
				borrowOrMerge(nodeId, childIdx);
			}
		}

		return removed;
	}

	void BPlusTree::borrowOrMerge(int64_t parentId, size_t idx) {
		std::vector<std::byte> parentPage(PAGE_SIZE);
		readPage(parentId, parentPage);
		auto* parentBuf = parentPage.data();
		uint16_t pn = pageNumKeys(parentBuf);

		int64_t leftId = (idx > 0) ? internalChild(parentBuf, idx - 1) : -1;
		int64_t rightId = (idx + 1 <= pn) ? internalChild(parentBuf, idx + 1) : -1;
		int64_t childId = internalChild(parentBuf, idx);

		std::vector<std::byte> childPage(PAGE_SIZE);
		readPage(childId, childPage);
		auto* childBuf = childPage.data();
		bool isLeaf = (pageType(childBuf) == 'L');

		// Read left/right pages early so pageBuffers are in scope for all paths
		std::vector<std::byte> leftPage(PAGE_SIZE);
		std::vector<std::byte> rightPage(PAGE_SIZE);
		auto* leftBuf = (leftId >= 0) ? (readPage(leftId, leftPage), leftPage.data()) : nullptr;
		auto* rightBuf = (rightId >= 0) ? (readPage(rightId, rightPage), rightPage.data()) : nullptr;

		// Try borrow from left sibling
		if (leftId >= 0 && leftBuf && pageNumKeys(leftBuf) > (isLeaf ? LEAF_MIN : INTERNAL_MIN)) {
			if (isLeaf) {
				std::string lastKey;
				BTreeLeafValue lastVal;
				leafGetEntry(leftBuf, pageNumKeys(leftBuf) - 1, lastKey, lastVal);

				std::string shiftKey;
				BTreeLeafValue shiftVal;
				for (uint16_t i = pageNumKeys(childBuf); i > 0; --i) {
					leafGetEntry(childBuf, i - 1, shiftKey, shiftVal);
					leafSetEntry(childBuf, i, shiftKey, shiftVal);
				}
				leafSetEntry(childBuf, 0, lastKey, lastVal);
				pageNumKeys(childBuf)++;
				pageNumKeys(leftBuf)--;
				std::string newSep;
				BTreeLeafValue newDummy;
				leafGetEntry(childBuf, 0, newSep, newDummy);
				std::memcpy(internalKeyPtr(parentBuf, idx), newSep.data(),
					std::min(newSep.size(), KEY_SIZE - 1));
			} else {
				auto parentKey = extractKey(internalKeyPtr(parentBuf, idx));
				for (uint16_t i = pageNumKeys(childBuf); i > 0; --i) {
					internalChild(childBuf, i) = internalChild(childBuf, i - 1);
					std::memcpy(internalKeyPtr(childBuf, i), internalKeyPtr(childBuf, i - 1), KEY_SIZE);
				}
				internalChild(childBuf, 0) = internalChild(leftBuf, pageNumKeys(leftBuf));
				uint16_t lk = pageNumKeys(leftBuf);
				std::memcpy(internalKeyPtr(childBuf, 1), internalKeyPtr(parentBuf, idx), KEY_SIZE);
				std::memcpy(internalKeyPtr(parentBuf, idx), internalKeyPtr(leftBuf, lk), KEY_SIZE);
				internalChild(childBuf, 1) = internalChild(leftBuf, lk);
				pageNumKeys(childBuf)++;
				pageNumKeys(leftBuf)--;
			}
			writePage(leftId, leftPage);
			writePage(childId, childPage);
			writePage(parentId, parentPage);
			return;
		}

		// Try borrow from right sibling
		if (rightId >= 0 && rightBuf && pageNumKeys(rightBuf) > (isLeaf ? LEAF_MIN : INTERNAL_MIN)) {
			if (isLeaf) {
				std::string firstKey;
				BTreeLeafValue firstVal;
				leafGetEntry(rightBuf, 0, firstKey, firstVal);
				uint16_t cn = pageNumKeys(childBuf);
				leafSetEntry(childBuf, cn, firstKey, firstVal);
				pageNumKeys(childBuf)++;
				for (uint16_t i = 0; i + 1 < pageNumKeys(rightBuf); ++i) {
					std::string shiftKey;
					BTreeLeafValue shiftValue;
					leafGetEntry(rightBuf, i + 1, shiftKey, shiftValue);
					leafSetEntry(rightBuf, i, shiftKey, shiftValue);
				}
				pageNumKeys(rightBuf)--;
				std::string newSep;
				BTreeLeafValue newDummy;
				leafGetEntry(rightBuf, 0, newSep, newDummy);
				std::memcpy(internalKeyPtr(parentBuf, idx + 1), newSep.data(),
					std::min(newSep.size(), KEY_SIZE - 1));
			} else {
				uint16_t cn = pageNumKeys(childBuf);
				internalChild(childBuf, cn + 1) = internalChild(rightBuf, 0);
				std::memcpy(internalKeyPtr(childBuf, cn + 1), internalKeyPtr(parentBuf, idx + 1), KEY_SIZE);
				std::memcpy(internalKeyPtr(parentBuf, idx + 1), internalKeyPtr(rightBuf, 1), KEY_SIZE);
				internalChild(rightBuf, 0) = internalChild(rightBuf, 1);
				for (uint16_t i = 1; i < pageNumKeys(rightBuf); ++i) {
					std::memcpy(internalKeyPtr(rightBuf, i), internalKeyPtr(rightBuf, i + 1), KEY_SIZE);
					internalChild(rightBuf, i) = internalChild(rightBuf, i + 1);
				}
				pageNumKeys(childBuf)++;
				pageNumKeys(rightBuf)--;
			}
			writePage(rightId, rightPage);
			writePage(childId, childPage);
			writePage(parentId, parentPage);
			return;
		}

		// Merge with a sibling
		if (leftId >= 0 && leftBuf) {
			if (isLeaf) {
				uint16_t ln = pageNumKeys(leftBuf);
				uint16_t cn = pageNumKeys(childBuf);
				for (uint16_t i = 0; i < cn; ++i) {
					std::string entryKey;
					BTreeLeafValue entryValue;
					leafGetEntry(childBuf, i, entryKey, entryValue);
					leafSetEntry(leftBuf, ln + i, entryKey, entryValue);
				}
				pageNumKeys(leftBuf) = ln + cn;
				pageNextLeaf(leftBuf) = pageNextLeaf(childBuf);
				for (uint16_t i = static_cast<uint16_t>(idx); i < pn; ++i) {
					std::memcpy(internalKeyPtr(parentBuf, i), internalKeyPtr(parentBuf, i + 1), KEY_SIZE);
					internalChild(parentBuf, i) = internalChild(parentBuf, i + 1);
				}
				pageNumKeys(parentBuf) = pn - 1;
			} else {
				uint16_t ln = pageNumKeys(leftBuf);
				std::memcpy(internalKeyPtr(leftBuf, ln + 1), internalKeyPtr(parentBuf, idx), KEY_SIZE);
				internalChild(leftBuf, ln + 1) = internalChild(childBuf, 0);
				uint16_t cn = pageNumKeys(childBuf);
				for (uint16_t i = 1; i <= cn; ++i) {
					std::memcpy(internalKeyPtr(leftBuf, ln + 1 + i), internalKeyPtr(childBuf, i), KEY_SIZE);
					internalChild(leftBuf, ln + 1 + i) = internalChild(childBuf, i);
				}
				pageNumKeys(leftBuf) = ln + 1 + cn;
				for (uint16_t i = static_cast<uint16_t>(idx); i < pn; ++i) {
					std::memcpy(internalKeyPtr(parentBuf, i), internalKeyPtr(parentBuf, i + 1), KEY_SIZE);
					internalChild(parentBuf, i) = internalChild(parentBuf, i + 1);
				}
				pageNumKeys(parentBuf) = pn - 1;
			}
			writePage(leftId, leftPage);
			writePage(parentId, parentPage);
		} else if (rightId >= 0 && rightBuf) {
			if (isLeaf) {
				uint16_t cn = pageNumKeys(childBuf);
				uint16_t rn = pageNumKeys(rightBuf);
				for (uint16_t i = rn; i > 0; --i) {
					std::string entryKey;
					BTreeLeafValue entryValue;
					leafGetEntry(rightBuf, i - 1, entryKey, entryValue);
					leafSetEntry(rightBuf, cn + i - 1, entryKey, entryValue);
				}
				for (uint16_t i = 0; i < cn; ++i) {
					std::string entryKey;
					BTreeLeafValue entryValue;
					leafGetEntry(childBuf, i, entryKey, entryValue);
					leafSetEntry(rightBuf, i, entryKey, entryValue);
				}
				pageNumKeys(rightBuf) = cn + rn;
				pagePrevLeaf(rightBuf) = pagePrevLeaf(childBuf);
				for (uint16_t i = static_cast<uint16_t>(idx + 1); i < pn; ++i) {
					std::memcpy(internalKeyPtr(parentBuf, i), internalKeyPtr(parentBuf, i + 1), KEY_SIZE);
					internalChild(parentBuf, i) = internalChild(parentBuf, i + 1);
				}
				pageNumKeys(parentBuf) = pn - 1;
			} else {
				uint16_t cn = pageNumKeys(childBuf);
				std::memcpy(internalKeyPtr(rightBuf, cn + 1), internalKeyPtr(parentBuf, idx + 1), KEY_SIZE);
				internalChild(rightBuf, cn + 1) = internalChild(rightBuf, 0);
				for (uint16_t i = 1; i <= cn; ++i) {
					std::memcpy(internalKeyPtr(rightBuf, cn + 1 + i), internalKeyPtr(childBuf, i), KEY_SIZE);
					internalChild(rightBuf, cn + 1 + i) = internalChild(childBuf, i);
				}
				internalChild(rightBuf, 0) = internalChild(childBuf, 0);
				pageNumKeys(rightBuf) = cn + 1 + pageNumKeys(rightBuf);
				for (uint16_t i = static_cast<uint16_t>(idx + 1); i < pn; ++i) {
					std::memcpy(internalKeyPtr(parentBuf, i), internalKeyPtr(parentBuf, i + 1), KEY_SIZE);
					internalChild(parentBuf, i) = internalChild(parentBuf, i + 1);
				}
				pageNumKeys(parentBuf) = pn - 1;
			}
			writePage(rightId, rightPage);
			writePage(parentId, parentPage);
		}
	}

} // namespace project_model

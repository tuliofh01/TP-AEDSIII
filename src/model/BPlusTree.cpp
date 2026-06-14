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
	// Internal node page layout (after 31-byte header):
	//   [child(0):8][key(1):20][child(1):8][key(2):20]...[child(N):8]
	//   ^-- HEADER_SIZE
	//                    ^-- HEADER_SIZE+CHILD_SIZE
	//                                   ^-- HEADER_SIZE+INTERNAL_ENTRY
	//                                                    ^-- HEADER_SIZE+INTERNAL_ENTRY+CHILD_SIZE
	//
	// Each INTERNAL_ENTRY = 28 bytes = CHILD_SIZE(8) + KEY_SIZE(20).
	// Child pointers and separator keys alternate:
	//   internalChild(buf, i)   = buf + HEADER_SIZE + i * INTERNAL_ENTRY
	//   internalKeyPtr(buf, i)  = buf + HEADER_SIZE + CHILD_SIZE + (i-1) * INTERNAL_ENTRY
	// ========================================
	int64_t& BPlusTree::internalChild(std::byte* pageBuf, size_t index) {
		return *reinterpret_cast<int64_t*>(pageBuf + HEADER_SIZE + index * INTERNAL_ENTRY);
	}

	std::byte* BPlusTree::internalKeyPtr(std::byte* pageBuf, size_t index) {
		return pageBuf + HEADER_SIZE + CHILD_SIZE + (index - 1) * INTERNAL_ENTRY;
	}

	// ========================================
	// Leaf node accessors
	// ========================================
	// Leaf node page layout (after 31-byte header):
	//   [entry(0):45][entry(1):45]...[entry(N):45]
	//   ^-- HEADER_SIZE
	//
	// Each LEAF_ENTRY = 45 bytes = KEY_SIZE(20) + LEAF_VALUE_SIZE(25).
	// leafEntryPtr(buf, i) = buf + HEADER_SIZE + i * LEAF_ENTRY
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
		// Format the key to the fixed KEY_SIZE, then descend from the root
		return searchInNode(fileHeader_.rootPageId, formatKey(key));
	}

	std::optional<BTreeLeafValue> BPlusTree::searchInNode(int64_t nodeId,
		const std::string& key) const {
		// Recursively descend the tree: at each internal node, scan separator keys
		// to find the correct child branch; at the leaf, scan entries for an exact match.
		std::vector<std::byte> pageBuffer(PAGE_SIZE);
		readPage(nodeId, pageBuffer);
		auto* pageBuf = pageBuffer.data();

		if (pageType(pageBuf) == 'L') {
			// Leaf: linear scan all entries looking for an exact key match
			uint16_t keyCount = pageNumKeys(pageBuf);
			for (uint16_t entryIndex = 0; entryIndex < keyCount; ++entryIndex) {
				std::string entryKey;
				BTreeLeafValue entryValue;
				leafGetEntry(pageBuf, entryIndex, entryKey, entryValue);
				if (entryKey == key) return entryValue;
			}
			return std::nullopt;
		}

		// Internal node: compare key against each separator to pick the correct child
		uint16_t keyCount = pageNumKeys(pageBuf);
		size_t childIndex = 0;
		for (uint16_t separatorIndex = 1; separatorIndex <= keyCount; ++separatorIndex) {
			auto* keyPtr = internalKeyPtr(pageBuf, separatorIndex);
			auto separatorKey = extractKey(keyPtr);
			if (key < separatorKey) {
				childIndex = static_cast<size_t>(separatorIndex - 1);
				goto descend_search;
			}
			childIndex = static_cast<size_t>(separatorIndex);
		}
		descend_search:
		int64_t childId = internalChild(pageBuf, childIndex);
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
		std::vector<std::pair<std::string, BTreeLeafValue>>& collectedResults) const {
		// Collect all entries whose key starts with the given prefix.
		// For internal nodes, recurse into every child (all branches potentially match).
		// For leaves, find the first entry >= prefix via linear scan,
		// then follow the next-leaf chain until keys no longer match the prefix.

		std::vector<std::byte> pageBuffer(PAGE_SIZE);
		readPage(nodeId, pageBuffer);
		auto* pageBuf = pageBuffer.data();

		if (pageType(pageBuf) == 'I') {
			uint16_t keyCount = pageNumKeys(pageBuf);
			// Internal node: every child branch may contain prefix-matching keys;
			// recurse into all of them (child[0] through child[keyCount])
			for (uint16_t childIndex = 0; childIndex <= keyCount; ++childIndex) {
				collectRange(internalChild(pageBuf, childIndex), prefix, collectedResults);
			}
			return;
		}

		// Leaf: scan entries in this leaf, then follow pageNextLeaf to the right
		int64_t currentLeafId = nodeId;
		bool rangeStarted = false;

		while (currentLeafId >= 0) {
			std::vector<std::byte> leafPageBuffer(PAGE_SIZE);
			readPage(currentLeafId, leafPageBuffer);
			auto* leafPageBuf = leafPageBuffer.data();
			uint16_t keyCount = pageNumKeys(leafPageBuf);

			for (uint16_t entryIndex = 0; entryIndex < keyCount; ++entryIndex) {
				std::string entryKey;
				BTreeLeafValue entryValue;
				leafGetEntry(leafPageBuf, entryIndex, entryKey, entryValue);

				if (!rangeStarted) {
					// Skip entries that are lexicographically smaller than the prefix
					if (entryKey.substr(0, prefix.size()) < prefix) continue;
					// Stop entirely if entries now exceed the prefix range
					if (entryKey.substr(0, prefix.size()) > prefix) return;
					rangeStarted = true;
				}

				// Once started, every entry must match the prefix; stop at the first mismatch
				if (entryKey.substr(0, prefix.size()) != prefix) return;
				collectedResults.emplace_back(entryKey, entryValue);
			}

			currentLeafId = pageNextLeaf(leafPageBuf);
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
			// ============================================================
			// Root-split: the single leaf root is full (≥ LEAF_MAX entries).
			// We transform it into a three-node tree:
			//
			//   Before:    [root: leaf (full)]
			//
			//   After:     [newRoot: internal]
			//              /                  \
			//   [oldRootId: leaf (1st half)]  [newLeafId: leaf (2nd half)]
			//
			// Key invariants:
			//   1. oldRootId keeps its page ID but is trimmed to the first half
			//   2. newLeafId is a fresh allocation for the second half
			//   3. newRootId is a fresh internal node (never reuse rootBuf)
			//   4. The separator key pushed up = first key of the new leaf
			//   5. Leaf chain: oldLeaf <-> newLeaf
			// ============================================================
			int64_t oldRootId = fileHeader_.rootPageId;
			int64_t newLeafId = allocatePage('L');
			int64_t newRootId = allocatePage('I');

			uint16_t totalKeys = pageNumKeys(rootBuf);
			uint16_t halfCount = totalKeys / 2;               // entries to keep in old leaf
			uint16_t movedCount = totalKeys - halfCount;       // entries to move to new leaf

			// Build new leaf buffer (right sibling, receives second half)
			std::vector<std::byte> newLeafBuffer(PAGE_SIZE, std::byte{0});
			pageType(newLeafBuffer.data()) = 'L';
			pagePrevLeaf(newLeafBuffer.data()) = oldRootId;
			pageNextLeaf(newLeafBuffer.data()) = pageNextLeaf(rootBuf);
			pageNextLeaf(rootBuf) = newLeafId;

			BTreeLeafValue movedValue;
			std::string movedKey;
			for (uint16_t i = 0; i < movedCount; ++i) {
				leafGetEntry(rootBuf, halfCount + i, movedKey, movedValue);
				leafSetEntry(newLeafBuffer.data(), i, movedKey, movedValue);
			}
			pageNumKeys(newLeafBuffer.data()) = movedCount;

			// Trim old leaf to first half
			pageNumKeys(rootBuf) = halfCount;

			// Extract separator key = first key of the new leaf
			leafGetEntry(newLeafBuffer.data(), 0, movedKey, movedValue);

			// Build new root internal node (fresh buffer, never alias rootBuf)
			std::vector<std::byte> newRootBuffer(PAGE_SIZE, std::byte{0});
			pageType(newRootBuffer.data()) = 'I';
			pageParent(newRootBuffer.data()) = -1;
			internalChild(newRootBuffer.data(), 0) = oldRootId;
			internalChild(newRootBuffer.data(), 1) = newLeafId;
			pageNumKeys(newRootBuffer.data()) = 1;
			std::memcpy(
				internalKeyPtr(newRootBuffer.data(), 1),
				movedKey.data(),
				std::min(movedKey.size(), KEY_SIZE - 1)
			);

			// Update parent pointers of children
			pageParent(rootBuf) = newRootId;
			pageParent(newLeafBuffer.data()) = newRootId;

			// Persist all three pages
			writePage(oldRootId, rootPage);        // trimmed leaf
			writePage(newLeafId, newLeafBuffer);    // new leaf
			writePage(newRootId, newRootBuffer);    // new internal root

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
		// Descend from nodeId, finding the correct leaf to insert into.
		// If a child is full, split it before descending further.
		// This guarantees every node along the path has room — hence "non-full".
		std::vector<std::byte> page(PAGE_SIZE);
		readPage(nodeId, page);
		auto* pageBuf = page.data();

		if (pageType(pageBuf) == 'L') {
			// Leaf: find sorted insertion position, shift entries right, insert
			uint16_t keyCount = pageNumKeys(pageBuf);
			int16_t insertionPosition = static_cast<int16_t>(keyCount) - 1;

			// Scan from right to left to find where key belongs
			while (insertionPosition >= 0) {
				std::string existingKey;
				BTreeLeafValue dummyVal;
				leafGetEntry(pageBuf, static_cast<size_t>(insertionPosition), existingKey, dummyVal);
				if (key >= existingKey) break;
				insertionPosition--;
			}
			insertionPosition++; // insert at insertionPosition (after the last smaller key)

			// Shift entries right to make room
			for (uint16_t i = keyCount; i > static_cast<uint16_t>(insertionPosition); --i) {
				std::string srcKey;
				BTreeLeafValue srcVal;
				leafGetEntry(pageBuf, i - 1, srcKey, srcVal);
				leafSetEntry(pageBuf, i, srcKey, srcVal);
			}

			leafSetEntry(pageBuf, static_cast<size_t>(insertionPosition), key, value);
			pageNumKeys(pageBuf) = keyCount + 1;
			writePage(nodeId, page);
			return;
		}

		// Internal node: find the correct child pointer by scanning separator keys
		uint16_t keyCount = pageNumKeys(pageBuf);
		size_t childIndex = 0;
		for (uint16_t i = 1; i <= keyCount; ++i) {
			auto* keyPtr = internalKeyPtr(pageBuf, i);
			auto separatorKey = extractKey(keyPtr);
			if (key < separatorKey) {
				childIndex = static_cast<size_t>(i - 1);
				goto descend_internal_nonfull;
			}
			childIndex = static_cast<size_t>(i);
		}
		descend_internal_nonfull:

		int64_t childId = internalChild(pageBuf, childIndex);

		// Read the child and check if it is full — if so, split it first
		std::vector<std::byte> childPageBuffer(PAGE_SIZE);
		readPage(childId, childPageBuffer);
		auto* childPageBuf = childPageBuffer.data();
		bool childIsFull = (pageType(childPageBuf) == 'L' && pageNumKeys(childPageBuf) >= LEAF_MAX)
			|| (pageType(childPageBuf) == 'I' && pageNumKeys(childPageBuf) >= INTERNAL_MAX);

		if (childIsFull) {
			splitChild(nodeId, childIndex, childId);
			// After split, re-read the parent (its keys/children shifted) and re-determine which child
			readPage(nodeId, page);
			pageBuf = page.data();
			keyCount = pageNumKeys(pageBuf);
			for (uint16_t i = 1; i <= keyCount; ++i) {
				auto* keyPtr = internalKeyPtr(pageBuf, i);
				auto separatorKey = extractKey(keyPtr);
				if (key < separatorKey) {
					childId = internalChild(pageBuf, static_cast<size_t>(i - 1));
					goto descend_after_split_nonfull;
				}
			}
			childId = internalChild(pageBuf, static_cast<size_t>(keyCount));
		}
		descend_after_split_nonfull:

		insertNonFull(childId, key, value);
	}

	void BPlusTree::splitChild(int64_t parentId, size_t childIndex, int64_t childId) {
		// Split a full child node into two siblings, promoting the median key
		// (or, for a leaf, the first key of the right sibling) into the parent.
		//
		// Internal node split layout:
		//   Before:        parent: [... child(i) ...]
		//                  child:  [c0][k1][c1][k2][c2]...[kN][cN]
		//
		//   After:         parent: [... child(i) | MEDIAN_KEY | newSibling ...]
		//                  child (left):  [c0][k1][c1]...[kH][cH]
		//                  newSibling:    [cH+1][kH+2][cH+2]...[kN][cN]
		//
		// Leaf split layout (no median, first key of right sibling promotes):
		//   After:         parent: [... child(i) | FIRST_KEY_OF_NEW ...]
		//                  child (left):  first half of entries
		//                  newSibling:    second half of entries
		// ============================================================

		std::vector<std::byte> parentPageBuffer(PAGE_SIZE);
		readPage(parentId, parentPageBuffer);
		auto* parentPageBuf = parentPageBuffer.data();

		std::vector<std::byte> childPageBuffer(PAGE_SIZE);
		readPage(childId, childPageBuffer);
		auto* childPageBuf = childPageBuffer.data();

		bool childIsLeaf = (pageType(childPageBuf) == 'L');
		int64_t newSiblingId = allocatePage(childIsLeaf ? 'L' : 'I');
		std::vector<std::byte> newSiblingBuffer(PAGE_SIZE, std::byte{0});
		auto* newSiblingBuf = newSiblingBuffer.data();

		pageParent(newSiblingBuf) = parentId;
		pageType(newSiblingBuf) = pageType(childPageBuf);

		uint16_t childKeyCount = pageNumKeys(childPageBuf);
		uint16_t halfCount = childIsLeaf ? LEAF_MAX / 2 : INTERNAL_MAX / 2;
		uint16_t entriesMoved = childKeyCount - halfCount;

		if (childIsLeaf) {
			// Copy the right half of the leaf entries into the new sibling
			for (uint16_t i = halfCount; i < childKeyCount; ++i) {
				std::string entryKey;
				BTreeLeafValue entryValue;
				leafGetEntry(childPageBuf, i, entryKey, entryValue);
				leafSetEntry(newSiblingBuf, i - halfCount, entryKey, entryValue);
			}
			pageNumKeys(newSiblingBuf) = entriesMoved;
			pageNumKeys(childPageBuf) = halfCount;

			// Link the two leaves into the doubly-linked chain
			pageNextLeaf(newSiblingBuf) = pageNextLeaf(childPageBuf);
			pagePrevLeaf(newSiblingBuf) = childId;
			pageNextLeaf(childPageBuf) = newSiblingId;

			// The separator key for a leaf split is the first key of the new sibling
			std::string splitKey;
			BTreeLeafValue dummyVal;
			leafGetEntry(newSiblingBuf, 0, splitKey, dummyVal);

			// Insert the new child pointer and separator key into the parent
			uint16_t parentKeyCount = pageNumKeys(parentPageBuf);
			for (uint16_t i = parentKeyCount; i > static_cast<uint16_t>(childIndex + 1); --i) {
				internalChild(parentPageBuf, i) = internalChild(parentPageBuf, i - 1);
				std::memcpy(
					internalKeyPtr(parentPageBuf, i),
					internalKeyPtr(parentPageBuf, i - 1),
					KEY_SIZE
				);
			}
			pageNumKeys(parentPageBuf) = parentKeyCount + 1;
			internalChild(parentPageBuf, childIndex + 1) = newSiblingId;
			std::memcpy(
				internalKeyPtr(parentPageBuf, childIndex + 1),
				splitKey.data(),
				std::min(splitKey.size(), KEY_SIZE - 1)
			);
		} else {
			// Internal node split: move the right half of children and keys
			// The median key (at position half + 1) is promoted to the parent.
			// Note: child[half] stays in the left node; child[half + 1] moves to the right node.

			// First child pointer of new sibling = child[half + 1] of original
			internalChild(newSiblingBuf, 0) = internalChild(childPageBuf, halfCount + 1);

			// Copy keys and child pointers for the right half (positions half+2 .. end)
			for (uint16_t i = halfCount + 1; i <= childKeyCount; ++i) {
				uint16_t destinationIndex = i - halfCount - 1;
				if (destinationIndex > 0) {
					std::memcpy(
						internalKeyPtr(newSiblingBuf, destinationIndex),
						internalKeyPtr(childPageBuf, i),
						KEY_SIZE
					);
				}
				internalChild(newSiblingBuf, destinationIndex + 1) = internalChild(childPageBuf, i + 1);
			}
			pageNumKeys(newSiblingBuf) = childKeyCount - halfCount - 1;
			pageNumKeys(childPageBuf) = halfCount;

			// The median key (at position half + 1 in the original child) goes up
			auto splitKey = extractKey(internalKeyPtr(childPageBuf, halfCount + 1));

			// Insert new child + separator key into parent, shifting right
			uint16_t parentKeyCount = pageNumKeys(parentPageBuf);
			for (uint16_t i = parentKeyCount; i > static_cast<uint16_t>(childIndex + 1); --i) {
				internalChild(parentPageBuf, i) = internalChild(parentPageBuf, i - 1);
				std::memcpy(
					internalKeyPtr(parentPageBuf, i),
					internalKeyPtr(parentPageBuf, i - 1),
					KEY_SIZE
				);
			}
			pageNumKeys(parentPageBuf) = parentKeyCount + 1;
			internalChild(parentPageBuf, childIndex + 1) = newSiblingId;
			std::memcpy(
				internalKeyPtr(parentPageBuf, childIndex + 1),
				splitKey.data(),
				std::min(splitKey.size(), KEY_SIZE - 1)
			);
		}

		writePage(childId, childPageBuffer);
		writePage(newSiblingId, newSiblingBuffer);
		writePage(parentId, parentPageBuffer);
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
		// Recursively find the target key in the tree and remove it.
		// If the resulting child is underfull (< LEAF_MIN or < INTERNAL_MIN),
		// trigger borrowOrMerge to rebalance from a sibling.
		std::vector<std::byte> pageBuffer(PAGE_SIZE);
		readPage(nodeId, pageBuffer);
		auto* pageBuf = pageBuffer.data();

		if (pageType(pageBuf) == 'L') {
			// Leaf: scan entries for an exact key match
			uint16_t keyCount = pageNumKeys(pageBuf);
			for (uint16_t entryIndex = 0; entryIndex < keyCount; ++entryIndex) {
				std::string entryKey;
				BTreeLeafValue entryValue;
				leafGetEntry(pageBuf, entryIndex, entryKey, entryValue);
				if (entryKey == key) {
					// Shift all subsequent entries left by one (overwrites the deleted entry)
					for (uint16_t shiftIndex = entryIndex; shiftIndex + 1 < keyCount; ++shiftIndex) {
						std::string nextKey;
						BTreeLeafValue nextValue;
						leafGetEntry(pageBuf, shiftIndex + 1, nextKey, nextValue);
						leafSetEntry(pageBuf, shiftIndex, nextKey, nextValue);
					}
					pageNumKeys(pageBuf) = keyCount - 1;
					writePage(nodeId, pageBuffer);
					return true;
				}
			}
			return false;
		}

		// Internal node: find which child branch leads to the key
		uint16_t keyCount = pageNumKeys(pageBuf);
		size_t childIndex = 0;
		for (uint16_t separatorIndex = 1; separatorIndex <= keyCount; ++separatorIndex) {
			auto* keyPtr = internalKeyPtr(pageBuf, separatorIndex);
			auto separatorKey = extractKey(keyPtr);
			if (key < separatorKey) {
				childIndex = static_cast<size_t>(separatorIndex - 1);
				goto erase_descend_internal;
			}
			childIndex = static_cast<size_t>(separatorIndex);
		}
		erase_descend_internal:

		int64_t childId = internalChild(pageBuf, childIndex);
		bool removed = eraseFromNode(childId, key);

		if (removed) {
			// Re-read the child and check if it is now underfull
			std::vector<std::byte> childPageBuffer(PAGE_SIZE);
			readPage(childId, childPageBuffer);
			auto* childPageBuf = childPageBuffer.data();
			size_t minimumKeys = (pageType(childPageBuf) == 'L') ? LEAF_MIN : INTERNAL_MIN;
			if (pageNumKeys(childPageBuf) < minimumKeys && nodeId != childId) {
				borrowOrMerge(nodeId, childIndex);
			}
		}

		return removed;
	}

	void BPlusTree::borrowOrMerge(int64_t parentId, size_t childIndex) {
		// Rebalance after a deletion. The child at childIndex is underfull.
		// Strategy (in order):
		//   1. Try to borrow one entry from the left sibling (if it has extras).
		//   2. Try to borrow one entry from the right sibling.
		//   3. Merge the underfull child with one of its siblings.
		//
		// After borrowing, the separator key in the parent is updated to reflect
		// the new boundary. After merging, the separator key is removed from the parent.
		std::vector<std::byte> parentPageBuffer(PAGE_SIZE);
		readPage(parentId, parentPageBuffer);
		auto* parentPageBuf = parentPageBuffer.data();
		uint16_t parentKeyCount = pageNumKeys(parentPageBuf);

		int64_t leftSiblingId = (childIndex > 0)
			? internalChild(parentPageBuf, childIndex - 1) : -1;
		int64_t rightSiblingId = (childIndex + 1 <= parentKeyCount)
			? internalChild(parentPageBuf, childIndex + 1) : -1;
		int64_t underfullChildId = internalChild(parentPageBuf, childIndex);

		std::vector<std::byte> childPageBuffer(PAGE_SIZE);
		readPage(underfullChildId, childPageBuffer);
		auto* childPageBuf = childPageBuffer.data();
		bool childIsLeaf = (pageType(childPageBuf) == 'L');

		// Read sibling pages now so buffers stay alive for all code paths
		std::vector<std::byte> leftSiblingBuffer(PAGE_SIZE);
		std::vector<std::byte> rightSiblingBuffer(PAGE_SIZE);
		auto* leftSiblingBuf = (leftSiblingId >= 0)
			? (readPage(leftSiblingId, leftSiblingBuffer), leftSiblingBuffer.data()) : nullptr;
		auto* rightSiblingBuf = (rightSiblingId >= 0)
			? (readPage(rightSiblingId, rightSiblingBuffer), rightSiblingBuffer.data()) : nullptr;

		uint16_t minimumKeys = childIsLeaf ? LEAF_MIN : INTERNAL_MIN;

		// ---------- Strategy 1: borrow from left sibling ----------
		if (leftSiblingId >= 0 && leftSiblingBuf
			&& pageNumKeys(leftSiblingBuf) > minimumKeys) {

			if (childIsLeaf) {
				// Move the last entry of left sibling to the front of this child
				std::string borrowedKey;
				BTreeLeafValue borrowedValue;
				leafGetEntry(leftSiblingBuf, pageNumKeys(leftSiblingBuf) - 1, borrowedKey, borrowedValue);

				// Shift all entries in child right by one
				std::string shiftedKey;
				BTreeLeafValue shiftedValue;
				for (uint16_t i = pageNumKeys(childPageBuf); i > 0; --i) {
					leafGetEntry(childPageBuf, i - 1, shiftedKey, shiftedValue);
					leafSetEntry(childPageBuf, i, shiftedKey, shiftedValue);
				}
				leafSetEntry(childPageBuf, 0, borrowedKey, borrowedValue);
				pageNumKeys(childPageBuf)++;
				pageNumKeys(leftSiblingBuf)--;

				// Update parent separator: new boundary = first key of child
				std::string newSeparatorKey;
				BTreeLeafValue dummyValue;
				leafGetEntry(childPageBuf, 0, newSeparatorKey, dummyValue);
				std::memcpy(
					internalKeyPtr(parentPageBuf, childIndex),
					newSeparatorKey.data(),
					std::min(newSeparatorKey.size(), KEY_SIZE - 1)
				);
			} else {
				// Borrow the rightmost child and separator key from left sibling
				auto parentSeparator = extractKey(internalKeyPtr(parentPageBuf, childIndex));

				// Shift all children/keys in the underfull node right
				for (uint16_t i = pageNumKeys(childPageBuf); i > 0; --i) {
					internalChild(childPageBuf, i) = internalChild(childPageBuf, i - 1);
					std::memcpy(
						internalKeyPtr(childPageBuf, i),
						internalKeyPtr(childPageBuf, i - 1),
						KEY_SIZE
					);
				}
				// Move left sibling's rightmost child to be child[0] of underfull node
				internalChild(childPageBuf, 0) = internalChild(leftSiblingBuf, pageNumKeys(leftSiblingBuf));

				uint16_t leftSiblingKeyCount = pageNumKeys(leftSiblingBuf);
				// Pull parent separator down as child's first key
				std::memcpy(
					internalKeyPtr(childPageBuf, 1),
					internalKeyPtr(parentPageBuf, childIndex),
					KEY_SIZE
				);
				// Push left sibling's rightmost key up to parent separator
				std::memcpy(
					internalKeyPtr(parentPageBuf, childIndex),
					internalKeyPtr(leftSiblingBuf, leftSiblingKeyCount),
					KEY_SIZE
				);
				internalChild(childPageBuf, 1) = internalChild(leftSiblingBuf, leftSiblingKeyCount);
				pageNumKeys(childPageBuf)++;
				pageNumKeys(leftSiblingBuf)--;
			}
			writePage(leftSiblingId, leftSiblingBuffer);
			writePage(underfullChildId, childPageBuffer);
			writePage(parentId, parentPageBuffer);
			return;
		}

		// ---------- Strategy 2: borrow from right sibling ----------
		if (rightSiblingId >= 0 && rightSiblingBuf
			&& pageNumKeys(rightSiblingBuf) > minimumKeys) {

			if (childIsLeaf) {
				// Move the first entry of right sibling to the end of this child
				std::string borrowedKey;
				BTreeLeafValue borrowedValue;
				leafGetEntry(rightSiblingBuf, 0, borrowedKey, borrowedValue);

				uint16_t childKeyCount = pageNumKeys(childPageBuf);
				leafSetEntry(childPageBuf, childKeyCount, borrowedKey, borrowedValue);
				pageNumKeys(childPageBuf)++;

				// Shift entries in right sibling left to fill the gap
				for (uint16_t i = 0; i + 1 < pageNumKeys(rightSiblingBuf); ++i) {
					std::string shiftedKey;
					BTreeLeafValue shiftedValue;
					leafGetEntry(rightSiblingBuf, i + 1, shiftedKey, shiftedValue);
					leafSetEntry(rightSiblingBuf, i, shiftedKey, shiftedValue);
				}
				pageNumKeys(rightSiblingBuf)--;

				// Update parent separator: new boundary = first key of right sibling
				std::string newSeparatorKey;
				BTreeLeafValue dummyValue;
				leafGetEntry(rightSiblingBuf, 0, newSeparatorKey, dummyValue);
				std::memcpy(
					internalKeyPtr(parentPageBuf, childIndex + 1),
					newSeparatorKey.data(),
					std::min(newSeparatorKey.size(), KEY_SIZE - 1)
				);
			} else {
				uint16_t childKeyCount = pageNumKeys(childPageBuf);
				// Move right sibling's leftmost child to end of child
				internalChild(childPageBuf, childKeyCount + 1) = internalChild(rightSiblingBuf, 0);
				// Pull parent separator down
				std::memcpy(
					internalKeyPtr(childPageBuf, childKeyCount + 1),
					internalKeyPtr(parentPageBuf, childIndex + 1),
					KEY_SIZE
				);
				// Push right sibling's leftmost key up to parent separator
				std::memcpy(
					internalKeyPtr(parentPageBuf, childIndex + 1),
					internalKeyPtr(rightSiblingBuf, 1),
					KEY_SIZE
				);
				internalChild(rightSiblingBuf, 0) = internalChild(rightSiblingBuf, 1);

				// Shift right sibling left to fill the gap
				for (uint16_t i = 1; i < pageNumKeys(rightSiblingBuf); ++i) {
					std::memcpy(
						internalKeyPtr(rightSiblingBuf, i),
						internalKeyPtr(rightSiblingBuf, i + 1),
						KEY_SIZE
					);
					internalChild(rightSiblingBuf, i) = internalChild(rightSiblingBuf, i + 1);
				}
				pageNumKeys(childPageBuf)++;
				pageNumKeys(rightSiblingBuf)--;
			}
			writePage(rightSiblingId, rightSiblingBuffer);
			writePage(underfullChildId, childPageBuffer);
			writePage(parentId, parentPageBuffer);
			return;
		}

		// ---------- Strategy 3: merge with a sibling ----------
		if (leftSiblingId >= 0 && leftSiblingBuf) {
			// Merge child into left sibling
			if (childIsLeaf) {
				uint16_t leftKeyCount = pageNumKeys(leftSiblingBuf);
				uint16_t childKeyCount = pageNumKeys(childPageBuf);
				// Append all child entries to left sibling
				for (uint16_t i = 0; i < childKeyCount; ++i) {
					std::string entryKey;
					BTreeLeafValue entryValue;
					leafGetEntry(childPageBuf, i, entryKey, entryValue);
					leafSetEntry(leftSiblingBuf, leftKeyCount + i, entryKey, entryValue);
				}
				pageNumKeys(leftSiblingBuf) = leftKeyCount + childKeyCount;
				// Update leaf chain: skip the now-merged child
				pageNextLeaf(leftSiblingBuf) = pageNextLeaf(childPageBuf);
				// Remove separator key and child pointer from parent (shift left)
				for (uint16_t i = static_cast<uint16_t>(childIndex); i < parentKeyCount; ++i) {
					std::memcpy(
						internalKeyPtr(parentPageBuf, i),
						internalKeyPtr(parentPageBuf, i + 1),
						KEY_SIZE
					);
					internalChild(parentPageBuf, i) = internalChild(parentPageBuf, i + 1);
				}
				pageNumKeys(parentPageBuf) = parentKeyCount - 1;
			} else {
				uint16_t leftKeyCount = pageNumKeys(leftSiblingBuf);
				// Pull parent separator down into left sibling
				std::memcpy(
					internalKeyPtr(leftSiblingBuf, leftKeyCount + 1),
					internalKeyPtr(parentPageBuf, childIndex),
					KEY_SIZE
				);
				internalChild(leftSiblingBuf, leftKeyCount + 1) = internalChild(childPageBuf, 0);

				uint16_t childKeyCount = pageNumKeys(childPageBuf);
				// Append all child keys and children to left sibling
				for (uint16_t i = 1; i <= childKeyCount; ++i) {
					std::memcpy(
						internalKeyPtr(leftSiblingBuf, leftKeyCount + 1 + i),
						internalKeyPtr(childPageBuf, i),
						KEY_SIZE
					);
					internalChild(leftSiblingBuf, leftKeyCount + 1 + i) = internalChild(childPageBuf, i);
				}
				pageNumKeys(leftSiblingBuf) = leftKeyCount + 1 + childKeyCount;
				// Remove separator and child pointer from parent
				for (uint16_t i = static_cast<uint16_t>(childIndex); i < parentKeyCount; ++i) {
					std::memcpy(
						internalKeyPtr(parentPageBuf, i),
						internalKeyPtr(parentPageBuf, i + 1),
						KEY_SIZE
					);
					internalChild(parentPageBuf, i) = internalChild(parentPageBuf, i + 1);
				}
				pageNumKeys(parentPageBuf) = parentKeyCount - 1;
			}
			writePage(leftSiblingId, leftSiblingBuffer);
			writePage(parentId, parentPageBuffer);
		} else if (rightSiblingId >= 0 && rightSiblingBuf) {
			// Merge child into right sibling
			if (childIsLeaf) {
				uint16_t childKeyCount = pageNumKeys(childPageBuf);
				uint16_t rightKeyCount = pageNumKeys(rightSiblingBuf);
				// Shift right sibling's entries right to make room at the front
				for (uint16_t i = rightKeyCount; i > 0; --i) {
					std::string entryKey;
					BTreeLeafValue entryValue;
					leafGetEntry(rightSiblingBuf, i - 1, entryKey, entryValue);
					leafSetEntry(rightSiblingBuf, childKeyCount + i - 1, entryKey, entryValue);
				}
				// Copy child entries into the front of right sibling
				for (uint16_t i = 0; i < childKeyCount; ++i) {
					std::string entryKey;
					BTreeLeafValue entryValue;
					leafGetEntry(childPageBuf, i, entryKey, entryValue);
					leafSetEntry(rightSiblingBuf, i, entryKey, entryValue);
				}
				pageNumKeys(rightSiblingBuf) = childKeyCount + rightKeyCount;
				// Update leaf chain backward pointer
				pagePrevLeaf(rightSiblingBuf) = pagePrevLeaf(childPageBuf);
				// Remove separator and child pointer from parent
				for (uint16_t i = static_cast<uint16_t>(childIndex + 1); i < parentKeyCount; ++i) {
					std::memcpy(
						internalKeyPtr(parentPageBuf, i),
						internalKeyPtr(parentPageBuf, i + 1),
						KEY_SIZE
					);
					internalChild(parentPageBuf, i) = internalChild(parentPageBuf, i + 1);
				}
				pageNumKeys(parentPageBuf) = parentKeyCount - 1;
			} else {
				uint16_t childKeyCount = pageNumKeys(childPageBuf);
				// Pull parent separator down into right sibling (shift right sibling's first child)
				std::memcpy(
					internalKeyPtr(rightSiblingBuf, childKeyCount + 1),
					internalKeyPtr(parentPageBuf, childIndex + 1),
					KEY_SIZE
				);
				internalChild(rightSiblingBuf, childKeyCount + 1) = internalChild(rightSiblingBuf, 0);

				// Prepend child's children and keys to right sibling
				for (uint16_t i = 1; i <= childKeyCount; ++i) {
					std::memcpy(
						internalKeyPtr(rightSiblingBuf, childKeyCount + 1 + i),
						internalKeyPtr(childPageBuf, i),
						KEY_SIZE
					);
					internalChild(rightSiblingBuf, childKeyCount + 1 + i) = internalChild(childPageBuf, i);
				}
				internalChild(rightSiblingBuf, 0) = internalChild(childPageBuf, 0);
				pageNumKeys(rightSiblingBuf) = childKeyCount + 1 + pageNumKeys(rightSiblingBuf);
				// Remove separator and child pointer from parent
				for (uint16_t i = static_cast<uint16_t>(childIndex + 1); i < parentKeyCount; ++i) {
					std::memcpy(
						internalKeyPtr(parentPageBuf, i),
						internalKeyPtr(parentPageBuf, i + 1),
						KEY_SIZE
					);
					internalChild(parentPageBuf, i) = internalChild(parentPageBuf, i + 1);
				}
				pageNumKeys(parentPageBuf) = parentKeyCount - 1;
			}
			writePage(rightSiblingId, rightSiblingBuffer);
			writePage(parentId, parentPageBuffer);
		}
	}

} // namespace project_model

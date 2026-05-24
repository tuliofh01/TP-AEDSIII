#include "BPlusTree.hpp"
#include <cstring>
#include <algorithm>

namespace project_model {

	// ========================================
	// BTreeFileHeader
	// ========================================
	auto BTreeFileHeader::toBytes() const -> std::vector<std::byte> {
		std::vector<std::byte> bytes(24);
		RecordSerializer::write(bytes.data(), 0, rootPageId);
		RecordSerializer::write(bytes.data(), 8, entryCount);
		RecordSerializer::write(bytes.data(), 16, pageCount);
		return bytes;
	}

	auto BTreeFileHeader::fromBytes(const std::vector<std::byte>& data) -> BTreeFileHeader {
		BTreeFileHeader hdr;
		if (data.size() >= 24) {
			auto* buf = data.data();
			hdr.rootPageId = RecordSerializer::read<int64_t>(buf, 0);
			hdr.entryCount = RecordSerializer::read<size_t>(buf, 8);
			hdr.pageCount = RecordSerializer::read<size_t>(buf, 16);
		}
		return hdr;
	}

	// ========================================
	// BPlusTree implementation
	// ========================================

	bool BPlusTree::initialize(std::fstream& file, size_t chunkOffset, size_t initialPages) {
		file_ = &file;
		chunkOffset_ = chunkOffset;

		// Try to read existing header
		loadHeader();

		// If no root, allocate initial pages
		if (fileHeader_.rootPageId < 0) {
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

	std::string BPlusTree::extractKey(const std::byte* buf) {
		return std::string(reinterpret_cast<const char*>(buf), KEY_SIZE).c_str();
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

	void BPlusTree::readPage(int64_t pageId, std::vector<std::byte>& buf) const {
		buf.resize(PAGE_SIZE);
		size_t offset = chunkOffset_ + CHUNK_HEADER + static_cast<size_t>(pageId) * PAGE_SIZE;
		file_->seekg(static_cast<std::streamoff>(offset), std::ios::beg);
		file_->read(reinterpret_cast<char*>(buf.data()), PAGE_SIZE);
	}

	void BPlusTree::writePage(int64_t pageId, const std::vector<std::byte>& buf) {
		size_t offset = chunkOffset_ + CHUNK_HEADER + static_cast<size_t>(pageId) * PAGE_SIZE;
		file_->seekp(static_cast<std::streamoff>(offset), std::ios::beg);
		file_->write(reinterpret_cast<const char*>(buf.data()), PAGE_SIZE);
	}

	void BPlusTree::saveHeader() {
		auto hdrBytes = fileHeader_.toBytes();
		file_->seekp(static_cast<std::streamoff>(chunkOffset_), std::ios::beg);
		file_->write(reinterpret_cast<const char*>(hdrBytes.data()), hdrBytes.size());
	}

	void BPlusTree::loadHeader() {
		std::vector<std::byte> hdrBytes(24);
		file_->seekg(static_cast<std::streamoff>(chunkOffset_), std::ios::beg);
		file_->read(reinterpret_cast<char*>(hdrBytes.data()), 24);
		fileHeader_ = BTreeFileHeader::fromBytes(hdrBytes);
	}

	// ========================================
	// Page field accessors
	// ========================================
	char& BPlusTree::pageType(std::byte* buf) {
		return *reinterpret_cast<char*>(buf);
	}

	uint16_t& BPlusTree::pageNumKeys(std::byte* buf) {
		return *reinterpret_cast<uint16_t*>(buf + 1);
	}

	int64_t& BPlusTree::pageParent(std::byte* buf) {
		return *reinterpret_cast<int64_t*>(buf + 3);
	}

	int64_t& BPlusTree::pageNextLeaf(std::byte* buf) {
		return *reinterpret_cast<int64_t*>(buf + 11);
	}

	int64_t& BPlusTree::pagePrevLeaf(std::byte* buf) {
		return *reinterpret_cast<int64_t*>(buf + 19);
	}

	// ========================================
	// Internal node accessors
	// ========================================
	int64_t& BPlusTree::internalChild(std::byte* buf, size_t index) {
		return *reinterpret_cast<int64_t*>(buf + HEADER_SIZE + index * INTERNAL_ENTRY);
	}

	std::byte* BPlusTree::internalKeyPtr(std::byte* buf, size_t index) {
		return buf + HEADER_SIZE + INTERNAL_ENTRY + (index - 1) * INTERNAL_ENTRY;
	}

	// ========================================
	// Leaf node accessors
	// ========================================
	std::byte* BPlusTree::leafEntryPtr(std::byte* buf, size_t index) {
		return buf + HEADER_SIZE + index * LEAF_ENTRY;
	}

	void BPlusTree::leafSetEntry(std::byte* buf, size_t index,
		const std::string& key, const BTreeLeafValue& value) {
		auto* entry = leafEntryPtr(buf, index);
		std::memset(entry, 0, LEAF_ENTRY);
		size_t copyLen = std::min(key.size(), KEY_SIZE - 1);
		std::memcpy(entry, key.data(), copyLen);
		auto valBytes = value.toBytes();
		std::memcpy(entry + KEY_SIZE, valBytes.data(), LEAF_VALUE_SIZE);
	}

	void BPlusTree::leafGetEntry(const std::byte* buf, size_t index,
		std::string& key, BTreeLeafValue& value) {
		auto* entry = buf + HEADER_SIZE + index * LEAF_ENTRY;
		key = std::string(reinterpret_cast<const char*>(entry), KEY_SIZE).c_str();
		std::vector<std::byte> valBytes(entry + KEY_SIZE, entry + KEY_SIZE + LEAF_VALUE_SIZE);
		value = BTreeLeafValue::fromBytes(valBytes);
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
		auto* buf = page.data();

		if (pageType(buf) == 'L') {
			// Leaf node: scan entries
			uint16_t n = pageNumKeys(buf);
			for (uint16_t i = 0; i < n; ++i) {
				std::string entryKey;
				BTreeLeafValue val;
				leafGetEntry(buf, i, entryKey, val);
				if (entryKey == key) return val;
			}
			return std::nullopt;
		}

		// Internal node: find correct child
		uint16_t n = pageNumKeys(buf);
		size_t childIdx = 0;
		for (uint16_t i = 1; i <= n; ++i) {
			std::string sepKey;
			BTreeLeafValue dummy;
			auto* kp = internalKeyPtr(buf, i);
			sepKey = extractKey(kp);
			if (key < sepKey) {
				childIdx = static_cast<size_t>(i - 1);
				goto descend;
			}
			childIdx = static_cast<size_t>(i);
		}
		descend:
		int64_t childId = internalChild(buf, childIdx);
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
		auto* buf = page.data();

		if (pageType(buf) == 'I') {
			uint16_t n = pageNumKeys(buf);
			// Find first child that might have matches
			collectRange(internalChild(buf, 0), prefix, out);
			for (uint16_t i = 1; i <= n; ++i) {
				collectRange(internalChild(buf, i), prefix, out);
			}
			return;
		}

		// Leaf: start from first matching entry, walk right
		int64_t currentId = nodeId;
		bool started = false;

		while (currentId >= 0) {
			std::vector<std::byte> leafPage(PAGE_SIZE);
			readPage(currentId, leafPage);
			auto* lbuf = leafPage.data();
			uint16_t n = pageNumKeys(lbuf);

			for (uint16_t i = 0; i < n; ++i) {
				std::string key;
				BTreeLeafValue val;
				leafGetEntry(lbuf, i, key, val);

				if (!started) {
					if (key.substr(0, prefix.size()) < prefix) continue;
					if (key.substr(0, prefix.size()) > prefix) return;
					started = true;
				}

				if (key.substr(0, prefix.size()) != prefix) return;
				out.emplace_back(key, val);
			}

			currentId = pageNextLeaf(lbuf);
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
		auto* buf = page.data();

		if (pageType(buf) == 'L') {
			// Leaf: insert in sorted position
			uint16_t n = pageNumKeys(buf);
			int16_t pos = static_cast<int16_t>(n) - 1;

			while (pos >= 0) {
				std::string existingKey;
				BTreeLeafValue dummyVal;
				leafGetEntry(buf, static_cast<size_t>(pos), existingKey, dummyVal);
				if (key >= existingKey) break;
				pos--;
			}
			pos++; // insert at pos+1

			// Shift entries right
			for (uint16_t i = n; i > static_cast<uint16_t>(pos); --i) {
				std::string srcKey;
				BTreeLeafValue srcVal;
				leafGetEntry(buf, i - 1, srcKey, srcVal);
				leafSetEntry(buf, i, srcKey, srcVal);
			}

			leafSetEntry(buf, static_cast<size_t>(pos), key, value);
			pageNumKeys(buf) = n + 1;
			writePage(nodeId, page);
			return;
		}

		// Internal: find child to descend into
		uint16_t n = pageNumKeys(buf);
		size_t childIdx = 0;
		for (uint16_t i = 1; i <= n; ++i) {
			auto* kp = internalKeyPtr(buf, i);
			std::string sepKey = extractKey(kp);
			if (key < sepKey) {
				childIdx = static_cast<size_t>(i - 1);
				goto descend_internal;
			}
			childIdx = static_cast<size_t>(i);
		}
		descend_internal:

		int64_t childId = internalChild(buf, childIdx);

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
			buf = page.data();
			n = pageNumKeys(buf);
			// Re-read the children
			for (uint16_t i = 1; i <= n; ++i) {
				auto* kp = internalKeyPtr(buf, i);
				std::string sepKey = extractKey(kp);
				if (key < sepKey) {
					childId = internalChild(buf, static_cast<size_t>(i - 1));
					goto descend_after_split;
				}
			}
			childId = internalChild(buf, static_cast<size_t>(n));
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
		std::vector<std::byte> newPage_buf(PAGE_SIZE);
		std::memset(newPage_buf.data(), 0, PAGE_SIZE);
		auto* newBuf = newPage_buf.data();

		pageParent(newBuf) = parentId;
		pageType(newBuf) = pageType(childBuf);

		uint16_t n = pageNumKeys(childBuf);
		uint16_t half = isLeaf ? LEAF_MAX / 2 : INTERNAL_MAX / 2;
		uint16_t moved = n - half;

		if (isLeaf) {
			// Copy right half to new leaf
			for (uint16_t i = half; i < n; ++i) {
				std::string ek;
				BTreeLeafValue ev;
				leafGetEntry(childBuf, i, ek, ev);
				leafSetEntry(newBuf, i - half, ek, ev);
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
			for (uint16_t i = half + 1; i <= n; ++i) {
				uint16_t newIdx = i - half - 1;
				if (newIdx > 0)
					std::memcpy(internalKeyPtr(newBuf, newIdx), internalKeyPtr(childBuf, i), KEY_SIZE);
				internalChild(newBuf, newIdx + 1) = internalChild(childBuf, i + 1);
			}
			pageNumKeys(newBuf) = n - half - 1;
			pageNumKeys(childBuf) = half;

			// The middle key goes up to parent
			std::string splitKey = extractKey(internalKeyPtr(childBuf, half + 1));

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
		writePage(newId, newPage_buf);
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
			auto* buf = rootPage.data();
			if (pageType(buf) == 'I' && pageNumKeys(buf) == 0 && fileHeader_.pageCount > 1) {
				fileHeader_.rootPageId = internalChild(buf, 0);
				pageParent(buf) = -1;
			}

			saveHeader();
			return true;
		}
		return false;
	}

	bool BPlusTree::eraseFromNode(int64_t nodeId, const std::string& key) {
		std::vector<std::byte> page(PAGE_SIZE);
		readPage(nodeId, page);
		auto* buf = page.data();

		if (pageType(buf) == 'L') {
			uint16_t n = pageNumKeys(buf);
			for (uint16_t i = 0; i < n; ++i) {
				std::string ek;
				BTreeLeafValue ev;
				leafGetEntry(buf, i, ek, ev);
				if (ek == key) {
					// Shift left
					for (uint16_t j = i; j + 1 < n; ++j) {
						std::string sjKey;
						BTreeLeafValue sjVal;
						leafGetEntry(buf, j + 1, sjKey, sjVal);
						leafSetEntry(buf, j, sjKey, sjVal);
					}
					pageNumKeys(buf) = n - 1;
					writePage(nodeId, page);
					return true;
				}
			}
			return false;
		}

		// Internal node
		uint16_t n = pageNumKeys(buf);
		size_t childIdx = 0;
		for (uint16_t i = 1; i <= n; ++i) {
			auto* kp = internalKeyPtr(buf, i);
			std::string sepKey = extractKey(kp);
			if (key < sepKey) {
				childIdx = static_cast<size_t>(i - 1);
				goto erase_descend;
			}
			childIdx = static_cast<size_t>(i);
		}
		erase_descend:

		int64_t childId = internalChild(buf, childIdx);
		bool removed = eraseFromNode(childId, key);

		if (removed) {
			// Check if child is underfull
			std::vector<std::byte> childPage(PAGE_SIZE);
			readPage(childId, childPage);
			auto* cbuf = childPage.data();
			size_t minKeys = (pageType(cbuf) == 'L') ? LEAF_MIN : INTERNAL_MIN;
			if (pageNumKeys(cbuf) < minKeys && nodeId != childId) {
				borrowOrMerge(nodeId, childIdx);
			}
		}

		return removed;
	}

	void BPlusTree::borrowOrMerge(int64_t parentId, size_t idx) {
		std::vector<std::byte> parentPage(PAGE_SIZE);
		readPage(parentId, parentPage);
		auto* pBuf = parentPage.data();
		uint16_t pn = pageNumKeys(pBuf);

		int64_t leftId = (idx > 0) ? internalChild(pBuf, idx - 1) : -1;
		int64_t rightId = (idx + 1 <= pn) ? internalChild(pBuf, idx + 1) : -1;
		int64_t childId = internalChild(pBuf, idx);

		std::vector<std::byte> childPage(PAGE_SIZE);
		readPage(childId, childPage);
		auto* cBuf = childPage.data();
		bool isLeaf = (pageType(cBuf) == 'L');

		// Read left/right pages early so buffers are in scope for all paths
		std::vector<std::byte> leftPage(PAGE_SIZE);
		std::vector<std::byte> rightPage(PAGE_SIZE);
		auto* lBuf = (leftId >= 0) ? (readPage(leftId, leftPage), leftPage.data()) : nullptr;
		auto* rBuf = (rightId >= 0) ? (readPage(rightId, rightPage), rightPage.data()) : nullptr;

		// Try borrow from left sibling
		if (leftId >= 0 && lBuf && pageNumKeys(lBuf) > (isLeaf ? LEAF_MIN : INTERNAL_MIN)) {
			if (isLeaf) {
				std::string lastKey;
				BTreeLeafValue lastVal;
				leafGetEntry(lBuf, pageNumKeys(lBuf) - 1, lastKey, lastVal);

				std::string shiftKey;
				BTreeLeafValue shiftVal;
				for (uint16_t i = pageNumKeys(cBuf); i > 0; --i) {
					leafGetEntry(cBuf, i - 1, shiftKey, shiftVal);
					leafSetEntry(cBuf, i, shiftKey, shiftVal);
				}
				leafSetEntry(cBuf, 0, lastKey, lastVal);
				pageNumKeys(cBuf)++;
				pageNumKeys(lBuf)--;
				std::string newSep;
				BTreeLeafValue newDummy;
				leafGetEntry(cBuf, 0, newSep, newDummy);
				std::memcpy(internalKeyPtr(pBuf, idx), newSep.data(),
					std::min(newSep.size(), KEY_SIZE - 1));
			} else {
				std::string parentKey = extractKey(internalKeyPtr(pBuf, idx));
				for (uint16_t i = pageNumKeys(cBuf); i > 0; --i) {
					internalChild(cBuf, i) = internalChild(cBuf, i - 1);
					std::memcpy(internalKeyPtr(cBuf, i), internalKeyPtr(cBuf, i - 1), KEY_SIZE);
				}
				internalChild(cBuf, 0) = internalChild(lBuf, pageNumKeys(lBuf));
				uint16_t lk = pageNumKeys(lBuf);
				std::memcpy(internalKeyPtr(cBuf, 1), internalKeyPtr(pBuf, idx), KEY_SIZE);
				std::memcpy(internalKeyPtr(pBuf, idx), internalKeyPtr(lBuf, lk), KEY_SIZE);
				internalChild(cBuf, 1) = internalChild(lBuf, lk);
				pageNumKeys(cBuf)++;
				pageNumKeys(lBuf)--;
			}
			writePage(leftId, leftPage);
			writePage(childId, childPage);
			writePage(parentId, parentPage);
			return;
		}

		// Try borrow from right sibling
		if (rightId >= 0 && rBuf && pageNumKeys(rBuf) > (isLeaf ? LEAF_MIN : INTERNAL_MIN)) {
			if (isLeaf) {
				std::string firstKey;
				BTreeLeafValue firstVal;
				leafGetEntry(rBuf, 0, firstKey, firstVal);
				uint16_t cn = pageNumKeys(cBuf);
				leafSetEntry(cBuf, cn, firstKey, firstVal);
				pageNumKeys(cBuf)++;
				for (uint16_t i = 0; i + 1 < pageNumKeys(rBuf); ++i) {
					std::string sk;
					BTreeLeafValue sv;
					leafGetEntry(rBuf, i + 1, sk, sv);
					leafSetEntry(rBuf, i, sk, sv);
				}
				pageNumKeys(rBuf)--;
				std::string newSep;
				BTreeLeafValue newDummy;
				leafGetEntry(rBuf, 0, newSep, newDummy);
				std::memcpy(internalKeyPtr(pBuf, idx + 1), newSep.data(),
					std::min(newSep.size(), KEY_SIZE - 1));
			} else {
				uint16_t cn = pageNumKeys(cBuf);
				internalChild(cBuf, cn + 1) = internalChild(rBuf, 0);
				std::memcpy(internalKeyPtr(cBuf, cn + 1), internalKeyPtr(pBuf, idx + 1), KEY_SIZE);
				std::memcpy(internalKeyPtr(pBuf, idx + 1), internalKeyPtr(rBuf, 1), KEY_SIZE);
				internalChild(rBuf, 0) = internalChild(rBuf, 1);
				for (uint16_t i = 1; i < pageNumKeys(rBuf); ++i) {
					std::memcpy(internalKeyPtr(rBuf, i), internalKeyPtr(rBuf, i + 1), KEY_SIZE);
					internalChild(rBuf, i) = internalChild(rBuf, i + 1);
				}
				pageNumKeys(cBuf)++;
				pageNumKeys(rBuf)--;
			}
			writePage(rightId, rightPage);
			writePage(childId, childPage);
			writePage(parentId, parentPage);
			return;
		}

		// Merge with a sibling
		if (leftId >= 0 && lBuf) {
			if (isLeaf) {
				uint16_t ln = pageNumKeys(lBuf);
				uint16_t cn = pageNumKeys(cBuf);
				for (uint16_t i = 0; i < cn; ++i) {
					std::string ek;
					BTreeLeafValue ev;
					leafGetEntry(cBuf, i, ek, ev);
					leafSetEntry(lBuf, ln + i, ek, ev);
				}
				pageNumKeys(lBuf) = ln + cn;
				pageNextLeaf(lBuf) = pageNextLeaf(cBuf);
				for (uint16_t i = static_cast<uint16_t>(idx); i < pn; ++i) {
					std::memcpy(internalKeyPtr(pBuf, i), internalKeyPtr(pBuf, i + 1), KEY_SIZE);
					internalChild(pBuf, i) = internalChild(pBuf, i + 1);
				}
				pageNumKeys(pBuf) = pn - 1;
			} else {
				uint16_t ln = pageNumKeys(lBuf);
				std::memcpy(internalKeyPtr(lBuf, ln + 1), internalKeyPtr(pBuf, idx), KEY_SIZE);
				internalChild(lBuf, ln + 1) = internalChild(cBuf, 0);
				uint16_t cn = pageNumKeys(cBuf);
				for (uint16_t i = 1; i <= cn; ++i) {
					std::memcpy(internalKeyPtr(lBuf, ln + 1 + i), internalKeyPtr(cBuf, i), KEY_SIZE);
					internalChild(lBuf, ln + 1 + i) = internalChild(cBuf, i);
				}
				pageNumKeys(lBuf) = ln + 1 + cn;
				for (uint16_t i = static_cast<uint16_t>(idx); i < pn; ++i) {
					std::memcpy(internalKeyPtr(pBuf, i), internalKeyPtr(pBuf, i + 1), KEY_SIZE);
					internalChild(pBuf, i) = internalChild(pBuf, i + 1);
				}
				pageNumKeys(pBuf) = pn - 1;
			}
			writePage(leftId, leftPage);
			writePage(parentId, parentPage);
		} else if (rightId >= 0 && rBuf) {
			if (isLeaf) {
				uint16_t cn = pageNumKeys(cBuf);
				uint16_t rn = pageNumKeys(rBuf);
				for (uint16_t i = rn; i > 0; --i) {
					std::string ek;
					BTreeLeafValue ev;
					leafGetEntry(rBuf, i - 1, ek, ev);
					leafSetEntry(rBuf, cn + i - 1, ek, ev);
				}
				for (uint16_t i = 0; i < cn; ++i) {
					std::string ek;
					BTreeLeafValue ev;
					leafGetEntry(cBuf, i, ek, ev);
					leafSetEntry(rBuf, i, ek, ev);
				}
				pageNumKeys(rBuf) = cn + rn;
				pagePrevLeaf(rBuf) = pagePrevLeaf(cBuf);
				for (uint16_t i = static_cast<uint16_t>(idx + 1); i < pn; ++i) {
					std::memcpy(internalKeyPtr(pBuf, i), internalKeyPtr(pBuf, i + 1), KEY_SIZE);
					internalChild(pBuf, i) = internalChild(pBuf, i + 1);
				}
				pageNumKeys(pBuf) = pn - 1;
			} else {
				uint16_t cn = pageNumKeys(cBuf);
				std::memcpy(internalKeyPtr(rBuf, cn + 1), internalKeyPtr(pBuf, idx + 1), KEY_SIZE);
				internalChild(rBuf, cn + 1) = internalChild(rBuf, 0);
				for (uint16_t i = 1; i <= cn; ++i) {
					std::memcpy(internalKeyPtr(rBuf, cn + 1 + i), internalKeyPtr(cBuf, i), KEY_SIZE);
					internalChild(rBuf, cn + 1 + i) = internalChild(cBuf, i);
				}
				internalChild(rBuf, 0) = internalChild(cBuf, 0);
				pageNumKeys(rBuf) = cn + 1 + pageNumKeys(rBuf);
				for (uint16_t i = static_cast<uint16_t>(idx + 1); i < pn; ++i) {
					std::memcpy(internalKeyPtr(pBuf, i), internalKeyPtr(pBuf, i + 1), KEY_SIZE);
					internalChild(pBuf, i) = internalChild(pBuf, i + 1);
				}
				pageNumKeys(pBuf) = pn - 1;
			}
			writePage(rightId, rightPage);
			writePage(parentId, parentPage);
		}
	}

} // namespace project_model

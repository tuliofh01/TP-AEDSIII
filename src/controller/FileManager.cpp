// FileManager implementation — binary file I/O with fixed-size records.
// Each record is identified by its zero-based index; byte offsets are
// derived as index * recordSize.

#include "FileManager.hpp"
#include "utility/Enums.hpp"
using namespace project_utility;

namespace project_controller {

	// Constructor: stores the fixed record size for offset calculation.
	FileManager::FileManager(size_t recordSize)
		: recordSize_(recordSize) {}

	// Opens or creates the binary file and loads the current record count.
	auto FileManager::initialize(const std::string& path) -> bool {
		filePath_ = path;
		fileStream_.open(filePath_, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
		if (!fileStream_.is_open()) {
			fileStream_.open(filePath_, std::ios::binary | std::ios::out | std::ios::trunc);
			if (!fileStream_.is_open()) return false;
		}
		updateCount();
		return true;
	}

	// Returns true if the underlying file stream is open.
	auto FileManager::isOpen() const -> bool {
		return fileStream_.is_open();
	}

	// Converts a record index to its byte offset in the file.
	auto FileManager::calcOffset(size_t index) const -> size_t {
		return index * recordSize_;
	}

	// Reads recordSize_ bytes at the given offset into buf.
	auto FileManager::readAt(std::vector<std::byte>& buf, size_t off) const -> bool {
		if (buf.size() < recordSize_) buf.resize(recordSize_);
		fileStream_.seekg(static_cast<std::streamoff>(off), std::ios::beg);
		if (!fileStream_.good()) return false;
		fileStream_.read(reinterpret_cast<char*>(buf.data()), recordSize_);
		return fileStream_.good() || fileStream_.eof();
	}

	// Writes recordSize_ bytes from data at the given byte offset.
	auto FileManager::writeAt(const std::vector<std::byte>& data, size_t off) const -> bool {
		if (data.size() < recordSize_) return false;
		fileStream_.seekp(static_cast<std::streamoff>(off), std::ios::beg);
		if (!fileStream_.good()) return false;
		fileStream_.write(reinterpret_cast<const char*>(data.data()), recordSize_);
		return fileStream_.good();
	}

	// Recalculates recordCount_ from the file size.
	void FileManager::updateCount() {
		fileStream_.seekg(0, std::ios::end);
		if (fileStream_.fail()) { recordCount_ = 0; return; }
		auto fileSize = fileStream_.tellg();
		recordCount_ = (fileSize > 0) ? static_cast<size_t>(fileSize) / recordSize_ : 0;
		fileStream_.seekg(0, std::ios::beg);
	}

	// Reads the record at the given index. Returns nullopt on failure.
	auto FileManager::readRecord(size_t index) const -> std::optional<std::vector<std::byte>> {
		std::vector<std::byte> buf(recordSize_);
		if (!readAt(buf, calcOffset(index))) return std::nullopt;
		return buf;
	}

	// Overwrites the record at the given index with data.
	auto FileManager::writeRecord(const std::vector<std::byte>& data, size_t index) -> bool {
		return writeAt(data, calcOffset(index));
	}

	// Appends a new record at the end of the file and increments the count.
	auto FileManager::appendRecord(const std::vector<std::byte>& data) -> bool {
		if (data.size() < recordSize_) return false;
		fileStream_.seekp(0, std::ios::end);
		if (!fileStream_.good()) return false;
		fileStream_.write(reinterpret_cast<const char*>(data.data()), recordSize_);
		if (!fileStream_.good()) return false;
		++recordCount_;
		return true;
	}

	// Counts records whose first byte marks them as active (non-deleted).
	auto FileManager::countActive() const -> uint32_t {
		using namespace project_utility;
		uint32_t count = 0;
		std::vector<std::byte> buf(recordSize_);
		for (size_t i = 0; i < recordCount_; ++i) {
			if (readAt(buf, calcOffset(i))) {
				auto s = std::to_integer<char>(buf[0]);
				if (s == static_cast<char>(RecStatus::Ativo)) ++count;
			}
		}
		return count;
	}

	// Returns the total number of records (including deleted ones).
	auto FileManager::countTotal() const -> uint32_t {
		return static_cast<uint32_t>(recordCount_);
	}

	// Sets the first byte of the record to the "deleted" sentinel value.
	auto FileManager::markDeleted(size_t index) const -> bool {
		if (index >= recordCount_) return false;
		std::vector<std::byte> buf(recordSize_);
		if (!readAt(buf, calcOffset(index))) return false;
		buf[0] = static_cast<std::byte>(RecStatus::Deletado);
		return writeAt(buf, calcOffset(index));
	}

	// Checks whether the record at index has been logically deleted.
	auto FileManager::isDeleted(size_t index) const -> bool {
		if (index >= recordCount_) return true;
		std::vector<std::byte> buf(recordSize_);
		if (!readAt(buf, calcOffset(index))) return true;
		return std::to_integer<char>(buf[0]) == static_cast<char>(RecStatus::Deletado);
	}

	// Flushes the file stream to disk.
	void FileManager::flush() const {
		if (fileStream_.is_open()) fileStream_.flush();
	}

	// Closes the file and resets the record count to zero.
	void FileManager::close() {
		if (fileStream_.is_open()) fileStream_.close();
		recordCount_ = 0;
	}

} // namespace project_controller

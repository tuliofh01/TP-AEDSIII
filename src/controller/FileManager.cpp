/**
 * @file FileManager.cpp
 * @brief Implementation of FileManager binary I/O methods.
 * @desc Implements all file operations: initialize, read, write, append,
 *       soft delete, and counting. Uses fstream for file access.
 * @author TP-AEDSIII Team
 * @date 2026-04-22 (backdated)
 * @namespace project_controller
 */
#include "FileManager.hpp"  // Class declaration
#include <algorithm>         // Not used but included for future

namespace project_controller {

	// ========================================
	// Method: initialize
	// Purpose: Opens or creates data file
	// ========================================
	/**
	 * @brief Opens file in binary read-write mode.
	 * @param path Path to .dat file.
	 * @return true if successful.
	 * @desc Tries to open in in|out|app mode. If fails, tries create mode.
	 *       Calls updateCount() to determine record count.
	 */
	auto FileManager::initialize(const std::string& path) -> bool {
		// Shortcut to project_utility constants
		using namespace project_utility;
		
		// Store path for later reference
		filePath_ = path;
		
		// Try to open existing file (read + write + append)
		fileStream_.open(filePath_, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
		
		// If file doesn't exist, create new one
		if (!fileStream_.is_open()) {
			fileStream_.open(filePath_, std::ios::binary | std::ios::out | std::ios::trunc);
			if (!fileStream_.is_open()) return false;
		}
		
		// Calculate how many records exist
		updateCount();
		return true;
	}

	/**
	 * @brief Checks if file stream is open.
	 */
	auto FileManager::isOpen() const -> bool {
		return fileStream_.is_open();
	}

	// ========================================
	// Method: calcOffset
	// Purpose: Calculate byte position for record
	// ========================================
	/**
	 * @brief Converts record index to byte offset.
	 * @param index Record number (0-based).
	 * @return Byte offset in file.
	 * @desc Multiplies index by RECORD_TOTAL_SIZE (67).
	 *       offset = index * 67
	 */
	auto FileManager::calcOffset(size_t index) const -> size_t {
		return index * project_utility::RECORD_TOTAL_SIZE;
	}

	// ========================================
	// Method: readAt
	// Purpose: Read block of bytes from file
	// ========================================
	/**
	 * @brief Reads exactly 67 bytes at given offset.
	 * @param buf Buffer to store data (auto-resizes if needed).
	 * @param off Byte offset from file start.
	 * @return true if read successful (even if at EOF).
	 */
	auto FileManager::readAt(std::vector<std::byte>& buf, size_t off) const -> bool {
		// Use namespace shortcut
		using namespace project_utility;
		
		// Ensure buffer is large enough
		if (buf.size() < RECORD_TOTAL_SIZE) buf.resize(RECORD_TOTAL_SIZE);
		
		// Seek to position
		fileStream_.seekg(static_cast<std::streamoff>(off), std::ios::beg);
		if (!fileStream_.good()) return false;
		
		// Read exactly 67 bytes
		fileStream_.read(reinterpret_cast<char*>(buf.data()), RECORD_TOTAL_SIZE);
		return fileStream_.good() || fileStream_.eof();
	}

	// ========================================
	// Method: writeAt
	// Purpose: Write block of bytes to file
	// ========================================
	/**
	 * @brief Writes exactly 67 bytes at given offset.
	 * @param data Byte vector to write.
	 * @param off Byte offset from file start.
	 * @return true if write successful.
	 */
	auto FileManager::writeAt(const std::vector<std::byte>& data, size_t off) const -> bool {
		// Use namespace shortcut
		using namespace project_utility;
		
		// Validate data size
		if (data.size() < RECORD_TOTAL_SIZE) return false;
		
		// Seek to position
		fileStream_.seekp(static_cast<std::streamoff>(off), std::ios::beg);
		if (!fileStream_.good()) return false;
		
		// Write exactly 67 bytes
		fileStream_.write(reinterpret_cast<const char*>(data.data()), RECORD_TOTAL_SIZE);
		return fileStream_.good();
	}

	// ========================================
	// Method: updateCount
	// Purpose: Calculate total records in file
	// ========================================
	/**
	 * @brief Updates recordCount_ from file size.
	 * @desc Seeks to end, gets position (file size), divides by 67.
	 */
	void FileManager::updateCount() {
		// Use namespace shortcut
		using namespace project_utility;
		
		// Go to end of file
		fileStream_.seekg(0, std::ios::end);
		if (fileStream_.fail()) { recordCount_ = 0; return; }
		
		// Get file size
		auto fileSize = fileStream_.tellg();
		
		// Calculate number of records
		recordCount_ = (fileSize > 0) ? static_cast<size_t>(fileSize) / RECORD_TOTAL_SIZE : 0;
		
		// Reset to beginning for next operation
		fileStream_.seekg(0, std::ios::beg);
	}

	// ========================================
	// Public read/write methods
	// ========================================
	
	/**
	 * @brief Reads record at specific index.
	 * @see readAt()
	 */
	auto FileManager::readRecord(size_t index) const -> std::optional<std::vector<std::byte>> {
		// Use namespace shortcut
		using namespace project_utility;
		
		std::vector<std::byte> buf(RECORD_TOTAL_SIZE);
		if (!readAt(buf, calcOffset(index))) return std::nullopt;
		return buf;
	}

	/**
	 * @brief Writes record at specific index.
	 * @see writeAt()
	 */
	auto FileManager::writeRecord(const std::vector<std::byte>& data, size_t index) -> bool {
		return writeAt(data, calcOffset(index));
	}

	/**
	 * @brief Appends new record at end of file.
	 * @desc Moves to EOF, writes data, increments count.
	 */
	auto FileManager::appendRecord(const std::vector<std::byte>& data) -> bool {
		// Use namespace shortcut
		using namespace project_utility;
		
		// Validate size
		if (data.size() < RECORD_TOTAL_SIZE) return false;
		
		// Go to end
		fileStream_.seekp(0, std::ios::end);
		if (!fileStream_.good()) return false;
		
		// Write data
		fileStream_.write(reinterpret_cast<const char*>(data.data()), RECORD_TOTAL_SIZE);
		if (!fileStream_.good()) return false;
		
		// Increment count
		++recordCount_;
		return true;
	}

	// ========================================
	// Counting methods
	// ========================================
	
	/**
	 * @brief Counts active records (status = 'A').
	 * @return Number of records NOT marked as deleted.
	 * @desc Iterates all records, checks status byte. Uses std::to_integer
	 *       to safely convert std::byte to char for comparison.
	 */
	auto FileManager::countActive() const -> uint32_t {
		// Use namespace shortcut
		using namespace project_utility;
		
		uint32_t count = 0;
		std::vector<std::byte> buf(RECORD_TOTAL_SIZE);
		
		// Scan all records
		for (size_t i = 0; i < recordCount_; ++i) {
			if (readAt(buf, calcOffset(i))) {
				// Convert std::byte to char safely
				auto s = std::to_integer<char>(buf[0]);
				if (s == static_cast<char>(RecStatus::Ativo)) ++count;
			}
		}
		return count;
	}

	/**
	 * @brief Returns total record count.
	 */
	auto FileManager::countTotal() const -> uint32_t {
		return static_cast<uint32_t>(recordCount_);
	}

	// ========================================
	// Soft delete methods
	// ========================================
	
	/**
	 * @brief Marks record as deleted (soft delete).
	 * @param index Record to mark.
	 * @return true if successful.
	 * @desc Reads record, changes status to '*', writes back.
	 */
	auto FileManager::markDeleted(size_t index) const -> bool {
		// Use namespace shortcut
		using namespace project_utility;
		
		// Bounds check
		if (index >= recordCount_) return false;
		
		std::vector<std::byte> buf(RECORD_TOTAL_SIZE);
		if (!readAt(buf, calcOffset(index))) return false;
		
		// Set status to '*' (Deletado)
		buf[0] = static_cast<std::byte>(RecStatus::Deletado);
		return writeAt(buf, calcOffset(index));
	}

	/**
	 * @brief Checks if record is deleted.
	 * @param index Record to check.
	 * @return true if deleted or out of range.
	 */
	auto FileManager::isDeleted(size_t index) const -> bool {
		// Use namespace shortcut
		using namespace project_utility;
		
		// Out of range = considered deleted
		if (index >= recordCount_) return true;
		
		std::vector<std::byte> buf(RECORD_TOTAL_SIZE);
		if (!readAt(buf, calcOffset(index))) return true;
		
		// Check status byte
		return std::to_integer<char>(buf[0]) == static_cast<char>(RecStatus::Deletado);
	}

	// ========================================
	// Cleanup methods
	// ========================================
	
	/**
	 * @brief Flushes write buffers to disk.
	 */
	void FileManager::flush() const {
		if (fileStream_.is_open()) fileStream_.flush();
	}

	/**
	 * @brief Closes file and resets count.
	 */
	void FileManager::close() {
		if (fileStream_.is_open()) fileStream_.close();
		recordCount_ = 0;
	}

} // namespace project_controller

/* ========================================================================
 * END OF FILE - FileManager.cpp
 * Purpose: Implement binary I/O for student records.
 * Dependencies: FileManager.hpp, Constants.hpp, Enums.hpp
 * Used By: DataManager.cpp, test_main.cpp
 * ======================================================================== */
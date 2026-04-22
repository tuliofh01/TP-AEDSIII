/**
 * @file FileManager.hpp
 * @brief Binary file I/O manager for student records.
 * @desc Handles all read/write operations on binary data file.
 *       Supports fixed-size records (67 bytes), soft delete, and
 *       counting operations. Uses fstream for file access.
 * @author TP-AEDSIII Team
 * @date 2026-04-22 (backdated)
 * @namespace project_controller
 */
#pragma once

// ========================================
// Standard library dependencies
// ========================================
#include <cstdint>     // int32_t, uint32_t, size_t
#include <string>      // std::string for path
#include <vector>      // std::vector<std::byte>
#include <optional>    // std::optional for return values
#include <fstream>     // std::fstream for file I/O

// ========================================
// Project dependencies (MVC modules)
// ========================================
#include "../model/Record.hpp"  // StudentRecord, RecordSerializer

/**
 * @namespace project_controller
 * @brief Flat namespace for business logic and I/O controllers.
 * @desc Contains classes that handle data operations: FileManager for
 *       binary I/O, IndexCtrl for hash indexing, DataManager for CRUD.
 */
namespace project_controller {

	/**
	 * @class FileManager
	 * @brief Manages binary file I/O for student records.
	 * @desc Responsibilities:
	 *       - Open/create data files
	 *       - Read/write 67-byte fixed records
	 *       - Track record count
	 *       - Perform soft delete (mark with '*')
	 *       - Count active records
	 * @note Each record is exactly RECORD_TOTAL_SIZE (67) bytes.
	 *       Records are appended to end of file.
	 * @see StudentRecord (67-byte layout)
	 * @see RecordSerializer (field serialization)
	 */
	class FileManager {
	public:
		// ========================================
		// Constructors and lifecycle
		// ========================================

		/** @brief Default constructor (no file opened yet). */
		FileManager() = default;

		/** @brief Destructor ensures file is closed. */
		~FileManager() = default;

		/** @brief Deleted copy constructor (file handles not copyable). */
		FileManager(const FileManager&) = delete;

		/** @brief Deleted copy assignment. */
		FileManager& operator=(const FileManager&) = delete;

		/** @brief Default move constructor. */
		FileManager(FileManager&&) noexcept = default;

		/** @brief Default move assignment. */
		FileManager& operator=(FileManager&&) noexcept = default;

		// ========================================
		// Public API methods
		// ========================================

		/**
		 * @brief Initializes/creates data file.
		 * @param path Full path to .dat file (e.g., "data/students.dat").
		 * @return true if file opened/created successfully, false on error.
		 * @desc Opens file in binary read-write mode. If file doesn't exist,
		 *       creates new empty file. Updates recordCount_.
		 * @example
		 *   FileManager fm;
		 *   if (!fm.initialize("data/students.dat")) {
		 *       std::cerr << "Failed to open file\n";
		 *   }
		 */
		[[nodiscard]] bool initialize(const std::string& path);

		/**
		 * @brief Checks if file is currently open.
		 * @return true if file is open and ready for I/O.
		 */
		[[nodiscard]] bool isOpen() const;

		/**
		 * @brief Reads record at given index.
		 * @param index Zero-based index (0 = first record).
		 * @return Optional vector of bytes, or nullopt if error/out of range.
		 * @desc Reads exactly RECORD_TOTAL_SIZE (67) bytes. Does NOT check
		 *       if record is deleted - caller must call isDeleted().
		 * @see isDeleted()
		 */
		[[nodiscard]] std::optional<std::vector<std::byte>> readRecord(size_t index) const;

		/**
		 * @brief Writes record at given index.
		 * @param data Byte vector (must have at least 67 bytes).
		 * @param index Zero-based index.
		 * @return true if write successful.
		 * @desc Overwrites existing record at index. Does NOT validate
		 *       that index is within bounds.
		 */
		[[nodiscard]] bool writeRecord(const std::vector<std::byte>& data, size_t index);

		/**
		 * @brief Appends new record to end of file.
		 * @param data Byte vector to append.
		 * @return true if append successful.
		 * @desc Moves file pointer to end, writes data, increments count.
		 * @note This is the primary way to add new students.
		 */
		[[nodiscard]] bool appendRecord(const std::vector<std::byte>& data);

		/**
		 * @brief Counts active records (status = 'A').
		 * @return Number of records with status != '*'.
		 * @desc Scans entire file, checks each status byte.
		 *       Slow for large files - use IndexCtrl for speed.
		 * @see IndexCtrl for O(1) lookup
		 */
		[[nodiscard]] uint32_t countActive() const;

		/**
		 * @brief Returns total record count in file.
		 * @return Total records including deleted ones.
		 * @desc Equal to file_size / RECORD_TOTAL_SIZE.
		 */
		[[nodiscard]] uint32_t countTotal() const;

		/**
		 * @brief Marks record as deleted (soft delete).
		 * @param index Zero-based index.
		 * @return true if mark successful.
		 * @desc Sets status byte to '*' (Deletado). Does NOT remove data.
		 *       This preserves data for potential recovery.
		 * @see isDeleted()
		 */
		[[nodiscard]] bool markDeleted(size_t index) const;

		/**
		 * @brief Checks if record is deleted.
		 * @param index Zero-based index.
		 * @return true if status == '*' or index out of range.
		 */
		[[nodiscard]] bool isDeleted(size_t index) const;

		/**
		 * @brief Flushes write buffers to disk.
		 * @desc Call after batch operations to ensure data is persisted.
		 */
		void flush() const;

		/**
		 * @brief Closes file and resets record count.
		 */
		void close();

	private:
		// ========================================
		// Private member variables
		// ========================================

		/** @brief File stream for reading/writing. */
		mutable std::fstream fileStream_;

		/** @brief Path to data file. */
		std::string filePath_;

		/** @brief Cached record count (updated on initialize). */
		size_t recordCount_ = 0;

		// ========================================
		// Private helper methods
		// ========================================

		/**
		 * @brief Calculates byte offset for record at given index.
		 * @param index Record index (0-based).
		 * @return Byte offset (index * 67).
		 */
		[[nodiscard]] size_t calcOffset(size_t index) const;

		/**
		 * @brief Reads block of bytes at specific offset.
		 * @param buf Output buffer (resized if needed).
		 * @param off Byte offset from start of file.
		 * @return true if read successful.
		 * @desc Uses seekg to position, then reads exactly 67 bytes.
		 */
		[[nodiscard]] bool readAt(std::vector<std::byte>& buf, size_t off) const;

		/**
		 * @brief Writes block of bytes at specific offset.
		 * @param data Data to write (must have 67+ bytes).
		 * @param off Byte offset from start of file.
		 * @return true if write successful.
		 */
		[[nodiscard]] bool writeAt(const std::vector<std::byte>& data, size_t off) const;

		/**
		 * @brief Updates recordCount_ based on file size.
		 * @desc Called during initialize to calculate how many records exist.
		 */
		void updateCount();
	};

} // namespace project_controller

/* ========================================================================
 * END OF FILE - FileManager.hpp
 * Purpose: Binary file I/O for 67-byte student records.
 * Dependencies: Record.hpp, <fstream>, <vector>, <optional>
 * Used By: DataManager.hpp/cpp
 * ======================================================================== */
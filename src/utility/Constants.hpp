/**
 * @file Constants.hpp
 * @brief Defines global constants for binary file layout and system config.
 * @desc This file contains all fixed-size values, offsets, and magic numbers
 *       needed for the binary persistence layer. Values are constexpr for
 *       compile-time evaluation and inlining.
 * @author TP-AEDSIII Team
 * @date 2026-04-22 (backdated)
 * @namespace project_utility - Flat namespace for constants/enums (MVC pattern).
 */
#pragma once

#include <cstdint>
#include <string_view>

/**
 * @namespace project_utility
 * @brief Flat namespace (no nesting) for utility types (MVC pattern).
 * @details Contains constants, enums, and helpers. Organized as flat namespace
 *          instead of nested (e.g., project::utility) for simplicity and 
 *          faster typing. All enum values can be accessed directly via
 *          'using enum' statement (C++20 feature).
 */
namespace project_utility {

	// ========================================
	// Fixed Layout Sizes (binary record)
	// ========================================
	/**
	 * @brief Maximum length of student name field (50 chars).
	 * @note Includes null terminator space (49 chars + null = 50).
	 * @sa RECORD_DATA_SIZE, RECORD_TOTAL_SIZE
	 */
	inline constexpr size_t NAME_LEN = 50;

	/**
	 * @brief Size of data portion (without status byte).
	 * @details Name(50) + id(4) + userId(4) + birthDate(4) + padding(4) = 66
	 */
	inline constexpr size_t RECORD_DATA_SIZE = 66;

	/**
	 * @brief Total size of one binary record (status byte + data).
	 * @details 1 (status) + 66 (data) = 67 bytes per record fixed.
	 */
	inline constexpr size_t RECORD_TOTAL_SIZE = 67;

	// ========================================
	// Field Offsets (byte positions in record)
	// ========================================
	/**
	 * @brief Offset where ID field starts (after status byte).
	 * @details Status byte at 0, so id starts at offset 1.
	 */
	inline constexpr size_t OFFSET_ID = 1;

	/**
	 * @brief Offset where user ID field starts.
	 * @details id occupies bytes 1-4, so userId starts at offset 5.
	 */
	inline constexpr size_t OFFSET_USER_ID = 5;

	/**
	 * @brief Offset where name field starts.
	 * @details id(4) + userId(4) = 8, plus status byte = 9.
	 */
	inline constexpr size_t OFFSET_NAME = 9;

	/**
	 * @brief Offset where birth date field starts.
	 * @details Name field is 50 bytes, starts at 9, so ends at 58-59.
	 *          Birth date at 59 (but we use 59 as start after name).
	 */
	inline constexpr size_t OFFSET_BIRTH_DATE = 59;

	// ========================================
	// File Extensions
	// ========================================
	/**
	 * @brief Extension for data files.
	 * @example students.dat
	 */
	inline constexpr std::string_view DATA_EXT = ".dat";

	/**
	 * @brief Extension for index files.
	 * @example students.idx
	 */
	inline constexpr std::string_view INDEX_EXT = ".idx";

	// ========================================
	// Index Rebuild Configuration
	// ========================================
	/**
	 * @brief Trigger rebuild after every N active records.
	 * @details Index rebuild is triggered when active record count is
	 *         exactly divisible by this value (modulo == 0).
	 * @example If REBUILD_MODULO=10, rebuild at 10, 20, 30... records.
	 * @sa IndexCtrl::shouldRebuild()
	 */
	inline constexpr uint32_t REBUILD_MODULO = 10;

	// ========================================
	// Index File Header Values
	// ========================================
	/**
	 * @brief Magic number for index file validation.
	 * @details ASCII 'INDE' = 0x494E4445. Used to verify file is valid.
	 *          Read at start of index file to confirm it's our format.
	 */
	inline constexpr uint32_t IDX_MAGIC = 0x494E4445u;

	/**
	 * @brief Initial hash table depth for index.
	 * @details Starting bucket count for hash table. Grows as needed.
	 *          Used in IndexCtrl constructor.
	 */
	inline constexpr uint32_t IDX_INITIAL_DEPTH = 2;

} // namespace project_utility

/* ========================================================================
 * END OF FILE - Constants.hpp
 * Purpose: Define all binary layout constants and magic values.
 * Dependencies: None (header-only with inline constexpr).
 * Used By: Record.hpp, FileManager.cpp, IndexCtrl.cpp, DataManager.cpp.
 * ======================================================================== */
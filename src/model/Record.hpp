/**
 * @file Record.hpp
 * @brief Data model for student records and binary serialization.
 * @desc Contains StudentRecord struct (67-byte fixed layout) and 
 *       RecordSerializer template for converting between C++ objects
 *       and binary data. Uses memcpy for portable serialization.
 * @author TP-AEDSIII Team
 * @date 2026-04-22 (backdated)
 * @namespace project_model - Flat namespace for data models.
 */
#pragma once

// ========================================
// Dependencies (standard library)
// ========================================
#include <cstdint>    // Fixed-width int types (int32_t, uint32_t)
#include <cstring>    // memcpy, memset, strlen
#include <string>     // std::string
#include <vector>     // std::vector<std::byte>

// ========================================
// Project dependencies (MVC modules)
// ========================================
#include "../utility/Constants.hpp"  // NAME_LEN, OFFSET_*, RECORD_TOTAL_SIZE
#include "../utility/Enums.hpp"       // RecStatus (using enum Ativo, Deletado)

/**
 * @namespace project_model
 * @brief Flat namespace for data models and serialization.
 * @desc Contains structs that represent data entities and utilities
 *       for converting between object and binary representations.
 */
namespace project_model {

	// ========================================
	// Record Serializer Template
	// ========================================
	/**
	 * @struct RecordSerializer
	 * @brief Utility for reading/writing fields to binary buffer.
	 * @desc Provides static template methods for serializing simple types
	 *       (int32_t, uint32_t, etc.) and fixed-length strings. Uses memcpy
	 *       which is the recommended approach for portable binary I/O
	 *       (avoids struct padding issues and endianness problems).
	 * @note For trivial types, std::bit_cast could be used (C++20), but
	 *       memcpy remains the consensus for cross-platform safety.
	 */
	struct RecordSerializer {
		// ========================================
		// Template: Write trivial field
		// ========================================
		/**
		 * @brief Writes a trivial (POD) type field to byte buffer.
		 * @tparam T Type to write (must be TriviallyCopyable).
		 * @param buf Pointer to output buffer (must have enough space).
		 * @param off Offset in bytes from start of buffer.
		 * @param val Value to write.
		 * @desc Uses std::memcpy which is safe and portable. For types
		 *       like int32_t, uint32_t, char, etc., this is ideal.
		 * @example
		 *   std::vector<std::byte> buf(67);
		 *   RecordSerializer::write<int32_t>(buf.data(), 1, 42);
		 */
		template<typename T>
		static void write(std::byte* buf, size_t off, const T& val) {
			std::memcpy(buf + off, &val, sizeof(T));
		}

		// ========================================
		// Template: Read trivial field
		// ========================================
		/**
		 * @brief Reads a trivial (POD) type field from byte buffer.
		 * @tparam T Type to read (must be TriviallyCopyable).
		 * @param buf Pointer to input buffer.
		 * @param off Offset in bytes from start of buffer.
		 * @return Value read from buffer.
		 * @desc Copies bytes from buffer to local variable using memcpy.
		 * @example
		 *   int32_t id = RecordSerializer::read<int32_t>(buf.data(), 1);
		 */
		template<typename T>
		static T read(const std::byte* buf, size_t off) {
			T val;
			std::memcpy(&val, buf + off, sizeof(T));
			return val;
		}

		// ========================================
		// String: Write fixed-length
		// ========================================
		/**
		 * @brief Writes null-terminated string to fixed-length buffer.
		 * @param buf Pointer to output buffer.
		 * @param off Offset in bytes.
		 * @param str Source string (may be nullptr or empty).
		 * @param len Maximum length (buffer size).
		 * @desc First clears buffer to zeros, then copies up to len-1 chars
		 *       from source string. Ensures null termination.
		 * @example
		 *   char name[50];
		 *   RecordSerializer::writeStr(buf.data(), 9, "Joao", 50);
		 */
		static void writeStr(std::byte* buf, size_t off, const char* str, size_t len) {
			std::memset(buf + off, 0, len);  // Clear buffer first
			if (str) {
				size_t copyLen = std::strlen(str) < len ? std::strlen(str) : len;
				std::memcpy(buf + off, str, copyLen);
			}
		}

		// ========================================
		// String: Read fixed-length
		// ========================================
		/**
		 * @brief Reads fixed-length buffer as std::string.
		 * @param buf Pointer to input buffer.
		 * @param off Offset in bytes.
		 * @param len Length of fixed field in bytes.
		 * @return std::string containing the read characters.
		 * @desc Copies raw bytes and truncates at null terminator if present.
		 * @example
		 *   std::string name = RecordSerializer::readStr(buf.data(), 9, 50);
		 */
		static std::string readStr(const std::byte* buf, size_t off, size_t len) {
			return std::string(reinterpret_cast<const char*>(buf + off), len).c_str();
		}
	};

	// ========================================
	// Student Record Struct
	// ========================================
	/**
	 * @struct StudentRecord
	 * @brief Student entity with fixed 67-byte binary layout.
	 * @desc Represents a single student in the system. Layout in memory:
	 *       - status (1 byte): 'A' = active, '*' = deleted
	 *       - id (4 bytes): Dynamic ID (recalculated on list)
	 *       - userId (4 bytes): External system ID
	 *       - name (50 bytes): Student name (fixed length)
	 *       - birthDate (4 bytes): Birth date as YYYYMMDD integer
	 *       - padding (4 bytes): Alignment padding
	 * @note Uses 'using enum RecStatus' to access Ativo/Deletado directly.
	 *       All fields use std:: types from <cstdint> for portability.
	 */
	struct StudentRecord {
		// ========================================
		// Import enum values directly (C++20)
		// ========================================
		/** @brief Import RecStatus values to struct scope. */
		using enum project_utility::RecStatus;

		// ========================================
		// Member fields (67 bytes total)
		// ========================================
		/** @brief Record status: 'A'=active, '*'=deleted (soft delete). */
		char status = static_cast<char>(Ativo);

		/** @brief Dynamic ID (recalculated each listing, starts at 1). */
		int32_t id = -1;

		/** @brief User ID from external system (not displayed). */
		int32_t userId = -1;

		/** @brief Student name (50 bytes, null-terminated). */
		char name[project_utility::NAME_LEN]{};

		/** @brief Birth date as integer YYYYMMDD (e.g., 15051990). */
		uint32_t birthDate = 0;

		// ========================================
		// Member functions
		// ========================================

		/**
		 * @brief Checks if record is active (not soft-deleted).
		 * @return true if status == 'A', false otherwise.
		 * @desc Simple inline check. Preferred over direct field access
		 *       as it encapsulates the logic in one place.
		 */
		[[nodiscard]] bool isActive() const {
			return status == static_cast<char>(Ativo);
		}

		/**
		 * @brief Marks record as active (status = 'A').
		 * @desc Sets status byte to 'A'. Used when creating new records
		 *       or undeleting existing ones.
		 */
		void setActive() { status = static_cast<char>(Ativo); }

		/**
		 * @brief Marks record as deleted (status = '*').
		 * @desc Performs soft delete - sets status to '*' without removing
		 *       data from file. This preserves data for potential recovery.
		 * @see FileManager::markDeleted()
		 */
		void setDeleted() { status = static_cast<char>(Deletado); }

		/**
		 * @brief Converts record to binary byte vector.
		 * @return std::vector<std::byte> of exactly RECORD_TOTAL_SIZE (67) bytes.
		 * @desc Serializes all fields in fixed order using RecordSerializer.
		 *       This is the inverse of fromBytes().
		 * @see fromBytes()
		 * @see RecordSerializer
		 */
		[[nodiscard]] std::vector<std::byte> toBytes() const;

		/**
		 * @brief Creates record from binary byte vector.
		 * @param data Vector of bytes (must have at least RECORD_TOTAL_SIZE).
		 * @return StudentRecord with fields populated from binary data.
		 * @desc Deserializes fields in fixed order. If data is too short,
		 *       returns default-initialized record (id=-1, status='A').
		 * @see toBytes()
		 */
		[[nodiscard]] static StudentRecord fromBytes(const std::vector<std::byte>& data);

		/**
		 * @brief Converts name array to std::string.
		 * @return std::string containing the name.
		 * @desc Helper to convert fixed char array to usable string.
		 *       Handles null termination properly.
		 */
		[[nodiscard]] std::string nameStr() const {
			return std::string(name, project_utility::NAME_LEN).c_str();
		}
	};

} // namespace project_model

/* ========================================================================
 * END OF FILE - Record.hpp
 * Purpose: Define StudentRecord (67 bytes) and RecordSerializer template.
 * Dependencies: Constants.hpp, Enums.hpp, <cstdint>, <cstring>, <string>, <vector>.
 * Used By: FileManager.cpp, DataManager.cpp, Record.cpp, test_main.cpp.
 * ======================================================================== */
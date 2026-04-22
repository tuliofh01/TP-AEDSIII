/**
 * @file Record.cpp
 * @brief Implementation of StudentRecord serialization methods.
 * @desc Implements toBytes() and fromBytes() using RecordSerializer.
 *       These methods handle conversion between C++ struct and binary format.
 * @author TP-AEDSIII Team
 * @date 2026-04-22 (backdated)
 * @namespace project_model
 */
#include "Record.hpp"  // StudentRecord, RecordSerializer
#include <cstring>     // std::strncpy

namespace project_model {

	// ========================================
	// Method: toBytes
	// Purpose: Serialize StudentRecord to binary vector (67 bytes)
	// ========================================
	/**
	 * @brief Converts StudentRecord to 67-byte binary representation.
	 * @return std::vector<std::byte> of exactly 67 bytes.
	 * @desc Writes fields in exact order:
	 *       1. status (1 byte) at offset 0
	 *       2. id (4 bytes) at offset 1
	 *       3. userId (4 bytes) at offset 5
	 *       4. name (50 bytes) at offset 9
	 *       5. birthDate (4 bytes) at offset 59
	 * @note Uses project_utility:: constants for offsets and sizes.
	 * @see fromBytes()
	 */
	auto StudentRecord::toBytes() const -> std::vector<std::byte> {
		// Use project_utility namespace for constants
		using namespace project_utility;

		// Create output buffer of exactly 67 bytes
		std::vector<std::byte> bytes(RECORD_TOTAL_SIZE);

		// Write each field using RecordSerializer (memcpy-based)
		RecordSerializer::write<char>(bytes.data(), 0, status);
		RecordSerializer::write(bytes.data(), OFFSET_ID, id);
		RecordSerializer::write(bytes.data(), OFFSET_USER_ID, userId);
		RecordSerializer::writeStr(bytes.data(), OFFSET_NAME, name, NAME_LEN);
		RecordSerializer::write(bytes.data(), OFFSET_BIRTH_DATE, birthDate);

		return bytes;
	}

	// ========================================
	// Method: fromBytes
	// Purpose: Deserialize binary vector to StudentRecord
	// ========================================
	/**
	 * @brief Creates StudentRecord from 67-byte binary data.
	 * @param data Vector of bytes (must have at least 67 bytes).
	 * @return StudentRecord with fields populated from binary.
	 * @desc Reads fields in exact same order as toBytes().
	 *       Returns default record if data is too short.
	 * @note Checks data.size() >= RECORD_TOTAL_SIZE before reading.
	 * @see toBytes()
	 */
	auto StudentRecord::fromBytes(const std::vector<std::byte>& data) -> StudentRecord {
		// Use project_utility namespace for constants
		using namespace project_utility;

		// Create default record (all fields set to default values)
		StudentRecord rec;

		// Only read if we have enough data (67+ bytes)
		if (data.size() >= RECORD_TOTAL_SIZE) {
			// Get raw pointer to data buffer
			auto* buf = data.data();

			// Read each field in same order as toBytes()
			rec.status = RecordSerializer::read<char>(buf, 0);
			rec.id = RecordSerializer::read<int32_t>(buf, OFFSET_ID);
			rec.userId = RecordSerializer::read<int32_t>(buf, OFFSET_USER_ID);
			rec.birthDate = RecordSerializer::read<uint32_t>(buf, OFFSET_BIRTH_DATE);

			// Read name string (50 bytes)
			auto s = RecordSerializer::readStr(buf, OFFSET_NAME, NAME_LEN);
			std::strncpy(rec.name, s.c_str(), NAME_LEN - 1);
			rec.name[NAME_LEN - 1] = '\0';  // Ensure null termination
		}

		return rec;
	}

} // namespace project_model

/* ========================================================================
 * END OF FILE - Record.cpp
 * Purpose: Implement toBytes() and fromBytes() serialization.
 * Dependencies: Record.hpp, <cstring>
 * Used By: FileManager.cpp, DataManager.cpp, test_main.cpp
 * ======================================================================== */
#pragma once

#include <cstdint>
#include <string_view>

namespace project_utility {

	// ========================================
	// Legacy constants (backward compat)
	// ========================================
	inline constexpr size_t NAME_LEN = 50;
	inline constexpr size_t RECORD_DATA_SIZE = 66;
	inline constexpr size_t RECORD_TOTAL_SIZE = 67;

	inline constexpr size_t OFFSET_ID = 1;
	inline constexpr size_t OFFSET_USER_ID = 5;
	inline constexpr size_t OFFSET_NAME = 9;
	inline constexpr size_t OFFSET_BIRTH_DATE = 59;

	// ========================================
	// Field lengths (new entities)
	// ========================================
	inline constexpr size_t EMAIL_LEN = 30;
	inline constexpr size_t CPF_LEN = 15;
	inline constexpr size_t COURSE_LEN = 30;
	inline constexpr size_t DEPT_LEN = 30;
	inline constexpr size_t SPEC_LEN = 30;
	inline constexpr size_t SUBJ_CODE_LEN = 20;
	inline constexpr size_t SEMESTER_LEN = 12;

	// ========================================
	// Record sizes (data portion, no status byte)
	// ========================================
	inline constexpr size_t USER_COMMON_SIZE = 1 + 1 + 4 + NAME_LEN + EMAIL_LEN + CPF_LEN;
	inline constexpr size_t STUDENT_EXTRA_SIZE = 4 + COURSE_LEN + 4;
	inline constexpr size_t TEACHER_EXTRA_SIZE = DEPT_LEN + SPEC_LEN + 4;
	inline constexpr size_t SUBJECT_DATA_SIZE = 1 + 1 + 4 + NAME_LEN + SUBJ_CODE_LEN + 4 + 4;

	inline constexpr size_t STUDENT_RECORD_SIZE = USER_COMMON_SIZE + STUDENT_EXTRA_SIZE;
	inline constexpr size_t TEACHER_RECORD_SIZE = USER_COMMON_SIZE + TEACHER_EXTRA_SIZE;
	inline constexpr size_t SUBJECT_RECORD_SIZE = SUBJECT_DATA_SIZE;

	// ========================================
	// Field offsets (UserRecord common)
	// ========================================
	inline constexpr size_t OFFSET_REC_TYPE = 1;
	inline constexpr size_t OFFSET_USER_ID2 = 2;
	inline constexpr size_t OFFSET_NAME2 = 6;
	inline constexpr size_t OFFSET_EMAIL = 56;
	inline constexpr size_t OFFSET_CPF = 86;

	// Student-specific offsets
	inline constexpr size_t OFFSET_BIRTH_DATE2 = 101;
	inline constexpr size_t OFFSET_COURSE = 105;
	inline constexpr size_t OFFSET_ENROLL_YEAR = 135;

	// Teacher-specific offsets
	inline constexpr size_t OFFSET_DEPT = 101;
	inline constexpr size_t OFFSET_SPEC = 131;
	inline constexpr size_t OFFSET_HIRE_DATE = 161;

	// Subject-specific offsets
	inline constexpr size_t SUBJ_OFFSET_TYPE = 1;
	inline constexpr size_t SUBJ_OFFSET_ID = 2;
	inline constexpr size_t SUBJ_OFFSET_NAME = 6;
	inline constexpr size_t SUBJ_OFFSET_CODE = 56;
	inline constexpr size_t SUBJ_OFFSET_CREDITS = 76;
	inline constexpr size_t SUBJ_OFFSET_TEACHER = 80;

	// ========================================
	// File header & chunk table
	// ========================================
	inline constexpr uint32_t FILE_MAGIC = 0x52454331u;
	inline constexpr uint32_t FILE_VERSION = 1;
	inline constexpr size_t FILE_HEADER_SIZE = 256;
	inline constexpr size_t CHUNK_INFO_SIZE = 32;
	inline constexpr uint32_t INITIAL_CHUNK_CAPACITY = 100;

	// ========================================
	// B+ Tree constants
	// ========================================
	inline constexpr size_t BTREE_PAGE_SIZE = 4096;
	inline constexpr size_t BTREE_HEADER_SIZE = 31;
	inline constexpr size_t BTREE_KEY_SIZE = 20;
	inline constexpr size_t BTREE_CHILD_SIZE = 8;
	inline constexpr size_t BTREE_INTERNAL_ENTRY_SIZE = BTREE_CHILD_SIZE + BTREE_KEY_SIZE;
	inline constexpr size_t BTREE_LEAF_VALUE_SIZE = 28;
	inline constexpr size_t BTREE_LEAF_ENTRY_SIZE = BTREE_KEY_SIZE + BTREE_LEAF_VALUE_SIZE;

	// Max keys per node
	inline constexpr size_t BTREE_INTERNAL_MAX_KEYS = (BTREE_PAGE_SIZE - BTREE_HEADER_SIZE - BTREE_CHILD_SIZE)
		/ BTREE_INTERNAL_ENTRY_SIZE;
	inline constexpr size_t BTREE_LEAF_MAX_KEYS = (BTREE_PAGE_SIZE - BTREE_HEADER_SIZE)
		/ BTREE_LEAF_ENTRY_SIZE;

	inline constexpr size_t BTREE_INTERNAL_MIN_KEYS = BTREE_INTERNAL_MAX_KEYS / 2;
	inline constexpr size_t BTREE_LEAF_MIN_KEYS = BTREE_LEAF_MAX_KEYS / 2;

	// ========================================
	// File extensions
	// ========================================
	inline constexpr std::string_view DATA_EXT = ".dat";
	inline constexpr std::string_view INDEX_EXT = ".idx";
	inline constexpr std::string_view DATA_FILE = "records.dat";
	inline constexpr std::string_view INDEX_FILE = "records.idx";

	// ========================================
	// Index magic & config
	// ========================================
	inline constexpr uint32_t IDX_MAGIC = 0x494E4445u;
	inline constexpr uint32_t IDX_INITIAL_DEPTH = 2;
	inline constexpr uint32_t REBUILD_MODULO = 10;

	// ========================================
	// Chunk type characters
	// ========================================
	inline constexpr char CHUNK_STUDENT = 'S';
	inline constexpr char CHUNK_TEACHER = 'T';
	inline constexpr char CHUNK_SUBJECT = 'B';
	inline constexpr char CHUNK_TREE = 'I';
	inline constexpr size_t CHUNK_COUNT = 4;

} // namespace project_utility

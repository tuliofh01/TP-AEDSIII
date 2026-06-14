#pragma once

#include <cstdint>
#include <string_view>

namespace project_utility {

// Fixed field sizes for packed binary records.
// Every record field has a compile-time constant width — no variable-length encoding.
// These must match between reads and writes; changing one breaks existing data files.
// ========================================
inline constexpr size_t NAME_LEN = 50;
inline constexpr size_t EMAIL_LEN = 30;
inline constexpr size_t CPF_LEN = 15;
inline constexpr size_t COURSE_LEN = 30;
inline constexpr size_t DEPT_LEN = 30;
inline constexpr size_t SPEC_LEN = 30;
inline constexpr size_t SUBJ_CODE_LEN = 20;
inline constexpr size_t SEMESTER_LEN = 12;

// Binary file layout: [FileHeader][ChunkTable][ChunkData...]
// FileHeader (256 bytes): magic, version, next-IDs, padding
// ChunkTable: 4 entries × 32 bytes describing each chunk's offset/size
// Chunk data: contiguous fixed-size records per entity type
// ========================================
inline constexpr uint32_t FILE_MAGIC = 0x52454331u;
inline constexpr uint32_t FILE_VERSION = 1;
inline constexpr size_t FILE_HEADER_SIZE = 256;
inline constexpr size_t CHUNK_INFO_SIZE = 32;
inline constexpr uint32_t INITIAL_CHUNK_CAPACITY = 100;

// B+ Tree layout: fixed 4096-byte pages in a dedicated chunk (type 'I').
// Leaf keys are 20 bytes (padded), values are sizeof(BTreeLeafValue) = 25 bytes.
// Internal keys link to child page ids (8 bytes each).
// Page geometry is computed at compile time in BPlusTree.hpp.
// ========================================
inline constexpr size_t BTREE_PAGE_SIZE = 4096;
inline constexpr size_t BTREE_HEADER_SIZE = 31;
inline constexpr size_t BTREE_KEY_SIZE = 20;
inline constexpr size_t BTREE_CHILD_SIZE = sizeof(int64_t);
inline constexpr size_t BTREE_INTERNAL_ENTRY_SIZE = BTREE_CHILD_SIZE + BTREE_KEY_SIZE;

inline constexpr size_t BTREE_INTERNAL_MAX_KEYS = (BTREE_PAGE_SIZE - BTREE_HEADER_SIZE - BTREE_CHILD_SIZE)
    / BTREE_INTERNAL_ENTRY_SIZE;
inline constexpr size_t BTREE_INTERNAL_MIN_KEYS = BTREE_INTERNAL_MAX_KEYS / 2;

// Grade is uint8_t (0–100). 60 is the passing threshold.
// This replaces the original float-based 0–10 scale: no rounding issues, no floating-point comparison.
// ========================================
inline constexpr uint8_t ACCEPTABLE_GRADE = 60;

// Two files per data directory: BASE_NAME + DATA_EXT for records,
// and BASE_NAME + INDEX_EXT for the hash-based CPF/ID index.
// ========================================
inline constexpr std::string_view BASE_NAME = "records";
inline constexpr std::string_view DATA_EXT = ".dat";
inline constexpr std::string_view INDEX_EXT = ".idx";

// IndexCtrl uses a std::unordered_map<string, IndexEntry> persisted to records.idx.
// IDX_MAGIC ('INDE') identifies the index file format.
// REBUILD_MODULO: every N writes triggers a full index rebuild from chunk data
// (handles stale entries from chunk reallocation without complex tombstone tracking).
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

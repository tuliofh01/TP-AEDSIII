#pragma once

// Record.hpp — packed binary record definitions + generic memcpy serialization.
//
// Every entity in the system (Student, Teacher, Subject, Enrollment) has a
// corresponding packed struct in this file. All structs are trivially copyable
// and #pragma packed to guarantee byte-exact layout — no padding, no alignment
// surprises. sizeof() gives the exact binary record size.
//
// Serialization model:
//   struct Foo { ... };                    // packed, trivially copyable
//   auto bytes = serializeRecord(foo);     // memcpy into vector<byte>
//   auto foo2  = deserializeRecord<Foo>(bytes);  // memcpy back
//
// This replaces the old per-field RecordSerializer with a single generic
// memcpy template — works for any trivially-copyable type.

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <type_traits>

#include "../utility/Constants.hpp"
#include "../utility/Enums.hpp"

namespace project_model {

// ========================================
// File Header (256 bytes) — always at offset 0 of records.dat
//
// Layout:
//   [0..3]   magic         = "REC1"
//   [4..7]   version       = 1
//   [8..11]  headerSize    = 256
//   [12..15] chunkCount    = 4
//   [16..19] nextStudentId = next auto-increment ID
//   [20..23] nextTeacherId
//   [24..27] nextSubjectId
//   [28..31] padding1
//   [32..255]reserved       = zeros
// ========================================
struct FileHeader {
    uint32_t magic = project_utility::FILE_MAGIC;
    uint32_t version = project_utility::FILE_VERSION;
    uint32_t headerSize = project_utility::FILE_HEADER_SIZE;
    uint32_t chunkCount = project_utility::CHUNK_COUNT;
    uint32_t nextStudentId = 1;
    uint32_t nextTeacherId = 1;
    uint32_t nextSubjectId = 1;
    uint32_t padding1 = 0;
    char reserved[project_utility::FILE_HEADER_SIZE - 32]{};

    [[nodiscard]] std::vector<std::byte> toBytes() const;
    static FileHeader fromBytes(const std::vector<std::byte>& data);
};

// ========================================
// Chunk Info (32 bytes each) — describes one contiguous block in records.dat
//
// After the file header comes a chunk table with 4 entries (S, T, B, I).
// Each entry says where the chunk starts (offset), how big each record is
// (recordSize), how many slots it can hold (capacity), and how many are in use
// (used). When used == capacity, the entire file is rewritten with double capacity.
// ========================================
struct ChunkInfo {
    char type = 0;       // 'S' student, 'T' teacher, 'B' subject, 'I' btree
    char pad1[3]{};
    uint64_t offset = 0;        // byte offset from start of records.dat
    uint32_t recordSize = 0;    // sizeof() for records in this chunk
    uint32_t capacity = project_utility::INITIAL_CHUNK_CAPACITY;
    uint32_t used = 0;
    uint32_t pad2 = 0;

    [[nodiscard]] std::vector<std::byte> toBytes() const;
    static ChunkInfo fromBytes(const std::vector<std::byte>& data);
};

// ========================================
// Packed binary records — zero padding between fields
// ========================================
#pragma pack(push, 1)

// Common user fields shared by StudentRecord and TeacherRecord.
// The first byte is always the status ('A' or '*'), allowing chunk scanning
// without knowing the concrete type: read 1 byte, skip if deleted.
struct UserRecord {
    using enum project_utility::RecStatus;

    char status = static_cast<char>(Ativo);     // 'A' active, '*' deleted
    char type = 0;                               // 'S' or 'T'
    int32_t userId = -1;
    uint16_t password = 0;                       // 16-bit numeric password
    char name[project_utility::NAME_LEN]{};      // null-terminated, fixed width
    char email[project_utility::EMAIL_LEN]{};
    char cpf[project_utility::CPF_LEN]{};

    [[nodiscard]] bool isActive() const {
        return status == static_cast<char>(Ativo);
    }

    void setActive() { status = static_cast<char>(Ativo); }
    void setDeleted() { status = static_cast<char>(Deletado); }

    // Helper: extract null-terminated C++ string from a fixed-width char array.
    // The trailing bytes may contain garbage after the first \0; constructing a
    // std::string from (ptr, maxLen) and then .c_str() ensures clean termination.
    [[nodiscard]] std::string nameStr() const {
        return std::string(name, project_utility::NAME_LEN).c_str();
    }

    [[nodiscard]] std::string emailStr() const {
        return std::string(email, project_utility::EMAIL_LEN).c_str();
    }

    [[nodiscard]] std::string cpfStr() const {
        return std::string(cpf, project_utility::CPF_LEN).c_str();
    }
};

// Student-specific fields appended after UserRecord.
// Total: sizeof(UserRecord fields) + 4 + 30 + 4 = 141 bytes.
struct StudentRecord : public UserRecord {
    uint32_t birthDate = 0;
    char courseName[project_utility::COURSE_LEN]{};
    int32_t enrollmentYear = 0;

    [[nodiscard]] std::string courseStr() const {
        return std::string(courseName, project_utility::COURSE_LEN).c_str();
    }
};

// Teacher-specific fields appended after UserRecord.
// Total: sizeof(UserRecord fields) + 30 + 30 + 4 = 167 bytes.
struct TeacherRecord : public UserRecord {
    char department[project_utility::DEPT_LEN]{};
    char specialization[project_utility::SPEC_LEN]{};
    uint32_t hireDate = 0;

    [[nodiscard]] std::string deptStr() const {
        return std::string(department, project_utility::DEPT_LEN).c_str();
    }

    [[nodiscard]] std::string specStr() const {
        return std::string(specialization, project_utility::SPEC_LEN).c_str();
    }
};

// SubjectRecord is independent (does not inherit UserRecord).
// It has its own status byte, ID, and teacherId foreign key.
// Total: 1 + 1 + 4 + 50 + 20 + 4 + 4 = 84 bytes.
struct SubjectRecord {
    using enum project_utility::RecStatus;

    char status = static_cast<char>(Ativo);
    char type = static_cast<char>(project_utility::RecType::Subject);
    int32_t subjectId = -1;
    char name[project_utility::NAME_LEN]{};
    char code[project_utility::SUBJ_CODE_LEN]{};
    int32_t credits = 0;
    int32_t teacherId = -1;     // FK to TeacherRecord.userId

    [[nodiscard]] bool isActive() const {
        return status == static_cast<char>(Ativo);
    }

    void setActive() { status = static_cast<char>(Ativo); }
    void setDeleted() { status = static_cast<char>(Deletado); }

    [[nodiscard]] std::string nameStr() const {
        return std::string(name, project_utility::NAME_LEN).c_str();
    }

    [[nodiscard]] std::string codeStr() const {
        return std::string(code, project_utility::SUBJ_CODE_LEN).c_str();
    }
};

// B+ Tree leaf value: stores enrollment data.
// Keys are formatted as "ENR:STU:0001:SUB:0001" for unique enrollment lookup.
// grade is uint8_t (0–100), isApproved() checks grade >= 60.
// Total: 4 + 4 + 4 + 1 + 12 = 25 bytes.
struct BTreeLeafValue {
    int32_t studentId = -1;
    int32_t subjectId = -1;
    int32_t teacherId = -1;
    uint8_t grade = 0;
    char semester[project_utility::SEMESTER_LEN]{};

    [[nodiscard]] bool isApproved() const {
        return grade >= project_utility::ACCEPTABLE_GRADE;
    }

    [[nodiscard]] std::string semesterStr() const {
        return std::string(semester, project_utility::SEMESTER_LEN).c_str();
    }
};

#pragma pack(pop)

// Compile-time checks: every packed struct must be trivially copyable
// (memcpy-safe) and match expected sizes. If a static_assert fires, either
// a field was added/removed without updating sizes, or #pragma pack was lost.
static_assert(std::is_trivially_copyable_v<StudentRecord>);
static_assert(std::is_trivially_copyable_v<TeacherRecord>);
static_assert(std::is_trivially_copyable_v<SubjectRecord>);
static_assert(std::is_trivially_copyable_v<BTreeLeafValue>);
static_assert(sizeof(StudentRecord) == 141);
static_assert(sizeof(TeacherRecord) == 167);
static_assert(sizeof(SubjectRecord) == 84);
static_assert(sizeof(BTreeLeafValue) == 25);

// ========================================
// Generic memcpy serialization
// ========================================
// write:  vector<byte> buf(sizeof(T)); memcpy(buf.data(), &rec, sizeof(T));
// read:   T rec; memcpy(&rec, buf.data(), min(buf.size(), sizeof(T)));
//
// Constraints:
//   - Only works for trivially copyable types (enforced by C++20 requires clause)
//   - No endianness conversion — assumes little-endian (x86, ARM in most configs)
//   - No pointer chasing — all data must be inline in the struct

template<typename T>
requires std::is_trivially_copyable_v<T>
inline std::vector<std::byte> serializeRecord(const T& rec) {
    std::vector<std::byte> buf(sizeof(T));
    std::memcpy(buf.data(), &rec, sizeof(T));
    return buf;
}

template<typename T>
requires std::is_trivially_copyable_v<T>
inline T deserializeRecord(const std::vector<std::byte>& buf) {
    T rec;
    std::memcpy(&rec, buf.data(),
        buf.size() < sizeof(T) ? buf.size() : sizeof(T));
    return rec;
}

} // namespace project_model

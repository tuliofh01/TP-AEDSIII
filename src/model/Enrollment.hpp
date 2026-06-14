#pragma once

// Enrollment.hpp — type-safe model wrapper around packed BTreeLeafValue.
//
// Represents the many-to-many relationship between students and subjects.
// Stores student ID, subject ID, teacher ID, numeric grade, approval status,
// and semester string. Provides serialize/fromBytes persistence and exposes
// the raw record for LuaStack delegation in the view layer.

#include "Record.hpp"
#include <cstring>
#include <string>
#include <vector>
#include <cstdint>

namespace project_model {

class Enrollment {
    BTreeLeafValue raw_;

public:
    Enrollment() = default;
    explicit Enrollment(const BTreeLeafValue& raw) : raw_(raw) {}

    int32_t getStudentId() const { return raw_.studentId; }
    void setStudentId(int32_t id) { raw_.studentId = id; }

    int32_t getSubjectId() const { return raw_.subjectId; }
    void setSubjectId(int32_t id) { raw_.subjectId = id; }

    int32_t getTeacherId() const { return raw_.teacherId; }
    void setTeacherId(int32_t id) { raw_.teacherId = id; }

    uint8_t getGrade() const { return raw_.grade; }
    void setGrade(uint8_t g) { raw_.grade = g; }

    bool isApproved() const { return raw_.isApproved(); }

    std::string getSemester() const { return raw_.semesterStr(); }
    void setSemester(const std::string& sem) {
        std::strncpy(raw_.semester, sem.c_str(), project_utility::SEMESTER_LEN - 1);
        raw_.semester[project_utility::SEMESTER_LEN - 1] = '\0';
    }

    // Expose raw packed struct for LuaStack delegation in main.cpp.
    // Lua views access fields via table keys (studentId, subjectId, teacherId, grade, semester, etc.)
    // which are set by LuaStack<BTreeLeafValue>::put().
    const BTreeLeafValue& raw() const { return raw_; }

    std::vector<std::byte> serialize() const {
        return serializeRecord(raw_);
    }

    static Enrollment fromBytes(const std::vector<std::byte>& bytes) {
        return Enrollment(deserializeRecord<BTreeLeafValue>(bytes));
    }
};

} // namespace project_model

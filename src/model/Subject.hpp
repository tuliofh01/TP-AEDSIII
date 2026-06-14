#pragma once

// Subject.hpp — type-safe model wrapper around packed SubjectRecord.
//
// Wraps a SubjectRecord with named getters/setters for the subject
// identifier, name, code, credit count, and assigned teacher ID.
// Provides serialize/fromBytes persistence and exposes the raw record
// for LuaStack delegation in the view layer.

#include "Record.hpp"
#include <cstring>
#include <string>
#include <vector>
#include <cstdint>

namespace project_model {

class Subject {
    SubjectRecord raw_;

public:
    Subject() = default;
    explicit Subject(const SubjectRecord& raw) : raw_(raw) {}

    int32_t getId() const { return raw_.subjectId; }
    void setId(int32_t id) { raw_.subjectId = id; }

    std::string getName() const { return raw_.nameStr(); }
    void setName(const std::string& name) {
        std::strncpy(raw_.name, name.c_str(), project_utility::NAME_LEN - 1);
        raw_.name[project_utility::NAME_LEN - 1] = '\0';
    }

    std::string getCode() const { return raw_.codeStr(); }
    void setCode(const std::string& code) {
        std::strncpy(raw_.code, code.c_str(), project_utility::SUBJ_CODE_LEN - 1);
        raw_.code[project_utility::SUBJ_CODE_LEN - 1] = '\0';
    }

    int32_t getCredits() const { return raw_.credits; }
    void setCredits(int32_t c) { raw_.credits = c; }

    int32_t getTeacherId() const { return raw_.teacherId; }
    void setTeacherId(int32_t id) { raw_.teacherId = id; }

    bool isActive() const { return raw_.isActive(); }
    void markDeleted() { raw_.setDeleted(); }
    void setActive() { raw_.setActive(); }

    // Expose raw packed struct for LuaStack delegation in main.cpp.
    // Lua views access fields via table keys (id, name, code, credits, teacherId, etc.)
    // which are set by LuaStack<SubjectRecord>::put().
    const SubjectRecord& raw() const { return raw_; }

    std::vector<std::byte> serialize() const {
        return serializeRecord(raw_);
    }

    static Subject fromBytes(const std::vector<std::byte>& bytes) {
        return Subject(deserializeRecord<SubjectRecord>(bytes));
    }
};

} // namespace project_model

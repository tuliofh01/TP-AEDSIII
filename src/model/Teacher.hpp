#pragma once

// Teacher.hpp — type-safe model wrapper around packed TeacherRecord.
//
// Wraps a TeacherRecord with named getters/setters for identification,
// contact info, academic department, specialization, and hire date.
// Provides serialize/fromBytes persistence and exposes the raw record
// for LuaStack delegation in the view layer.

#include "Record.hpp"
#include <cstring>
#include <string>
#include <vector>
#include <cstdint>

namespace project_model {

class Teacher {
    TeacherRecord raw_;

public:
    Teacher() = default;
    explicit Teacher(const TeacherRecord& raw) : raw_(raw) {}

    int32_t getId() const { return raw_.userId; }
    void setId(int32_t id) { raw_.userId = id; }

    uint16_t getPassword() const { return raw_.password; }
    void setPassword(uint16_t pw) { raw_.password = pw; }

    std::string getName() const { return raw_.nameStr(); }
    void setName(const std::string& name) {
        std::strncpy(raw_.name, name.c_str(), project_utility::NAME_LEN - 1);
        raw_.name[project_utility::NAME_LEN - 1] = '\0';
    }

    std::string getEmail() const { return raw_.emailStr(); }
    void setEmail(const std::string& email) {
        std::strncpy(raw_.email, email.c_str(), project_utility::EMAIL_LEN - 1);
        raw_.email[project_utility::EMAIL_LEN - 1] = '\0';
    }

    std::string getCpf() const { return raw_.cpfStr(); }
    void setCpf(const std::string& cpf) {
        std::strncpy(raw_.cpf, cpf.c_str(), project_utility::CPF_LEN - 1);
        raw_.cpf[project_utility::CPF_LEN - 1] = '\0';
    }

    std::string getDepartment() const { return raw_.deptStr(); }
    void setDepartment(const std::string& dept) {
        std::strncpy(raw_.department, dept.c_str(), project_utility::DEPT_LEN - 1);
        raw_.department[project_utility::DEPT_LEN - 1] = '\0';
    }

    std::string getSpecialization() const { return raw_.specStr(); }
    void setSpecialization(const std::string& spec) {
        std::strncpy(raw_.specialization, spec.c_str(), project_utility::SPEC_LEN - 1);
        raw_.specialization[project_utility::SPEC_LEN - 1] = '\0';
    }

    uint32_t getHireDate() const { return raw_.hireDate; }
    void setHireDate(uint32_t date) { raw_.hireDate = date; }

    bool isActive() const { return raw_.isActive(); }
    void markDeleted() { raw_.setDeleted(); }
    void setActive() { raw_.setActive(); }

    // Expose raw packed struct for LuaStack delegation in main.cpp.
    // Lua views access fields via table keys (id, name, email, cpf, dept, spec, etc.)
    // which are set by LuaStack<TeacherRecord>::put().
    const TeacherRecord& raw() const { return raw_; }

    std::vector<std::byte> serialize() const {
        return serializeRecord(raw_);
    }

    static Teacher fromBytes(const std::vector<std::byte>& bytes) {
        return Teacher(deserializeRecord<TeacherRecord>(bytes));
    }
};

} // namespace project_model

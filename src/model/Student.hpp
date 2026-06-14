#pragma once

// Student.hpp — type-safe model wrapper around packed StudentRecord.
//
// The model layer wraps raw packed structs via composition (not inheritance).
// Each model class provides:
//   - Named getters/setters with std::string conversion
//   - serialize() → vector<byte> for persistence
//   - static fromBytes() → model for deserialization
//   - .raw() → const reference to underlying packed record (for LuaStack delegation)
//
// This separation keeps the packed structs dumb data carriers while the model
// classes provide the public API that both C++ controllers and Lua bindings use.

#include "Record.hpp"
#include <cstring>
#include <string>
#include <vector>
#include <cstdint>

namespace project_model {

class Student {
    StudentRecord raw_;

public:
    Student() = default;
    explicit Student(const StudentRecord& raw) : raw_(raw) {}

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

    uint32_t getBirthDate() const { return raw_.birthDate; }
    void setBirthDate(uint32_t date) { raw_.birthDate = date; }

    std::string getCourse() const { return raw_.courseStr(); }
    void setCourse(const std::string& course) {
        std::strncpy(raw_.courseName, course.c_str(), project_utility::COURSE_LEN - 1);
        raw_.courseName[project_utility::COURSE_LEN - 1] = '\0';
    }

    int32_t getEnrollmentYear() const { return raw_.enrollmentYear; }
    void setEnrollmentYear(int32_t year) { raw_.enrollmentYear = year; }

    bool isActive() const { return raw_.isActive(); }
    void markDeleted() { raw_.setDeleted(); }
    void setActive() { raw_.setActive(); }

    // Expose raw packed struct for LuaStack delegation in main.cpp.
    // Lua views access fields via table keys (id, name, email, cpf, etc.)
    // which are set by LuaStack<StudentRecord>::put().
    const StudentRecord& raw() const { return raw_; }

    std::vector<std::byte> serialize() const {
        return serializeRecord(raw_);
    }

    static Student fromBytes(const std::vector<std::byte>& bytes) {
        return Student(deserializeRecord<StudentRecord>(bytes));
    }
};

} // namespace project_model

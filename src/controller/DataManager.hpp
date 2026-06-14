#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <fstream>

#include "FileManager.hpp"
#include "IndexCtrl.hpp"
#include "../model/Record.hpp"
#include "../model/BPlusTree.hpp"
#include "../model/Student.hpp"
#include "../model/Teacher.hpp"
#include "../model/Subject.hpp"
#include "../model/Enrollment.hpp"

namespace project_controller {

struct LoginResult {
    int32_t userId = -1;
    char role = 0;
    std::string name;
};

class DataManager {
public:
	DataManager() = default;
	~DataManager();

    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;

    [[nodiscard]] bool initialize(const std::string& dataDir);

    // ---- Student CRUD ----
    [[nodiscard]] bool createStudent(
        const std::string& name, const std::string& email,
        const std::string& cpf, uint16_t password, uint32_t birthDate,
        const std::string& courseName, int32_t enrollmentYear);
    [[nodiscard]] std::optional<project_model::Student> readStudent(int32_t id) const;
    [[nodiscard]] bool deleteStudent(int32_t id);
    [[nodiscard]] bool updateStudent(int32_t id, const project_model::StudentRecord& updated);
    [[nodiscard]] std::vector<project_model::Student> listAllStudents() const;

    // ---- Teacher CRUD ----
    [[nodiscard]] bool createTeacher(
        const std::string& name, const std::string& email,
        const std::string& cpf, uint16_t password,
        const std::string& department,
        const std::string& specialization, uint32_t hireDate);
    [[nodiscard]] std::optional<project_model::Teacher> readTeacher(int32_t id) const;
    [[nodiscard]] bool deleteTeacher(int32_t id);
    [[nodiscard]] bool updateTeacher(int32_t id, const project_model::TeacherRecord& updated);
    [[nodiscard]] std::vector<project_model::Teacher> listAllTeachers() const;

    // ---- Subject CRUD ----
    [[nodiscard]] bool createSubject(
        const std::string& name, const std::string& code,
        int32_t credits, int32_t teacherId);
    [[nodiscard]] std::optional<project_model::Subject> readSubject(int32_t id) const;
    [[nodiscard]] bool deleteSubject(int32_t id);
    [[nodiscard]] bool updateSubject(int32_t id, const project_model::SubjectRecord& updated);
    [[nodiscard]] std::vector<project_model::Subject> listAllSubjects() const;

    // ---- Enrollment (via B+ Tree) ----
    [[nodiscard]] bool enrollStudent(
        int32_t studentId, int32_t subjectId,
        int32_t teacherId, const std::string& semester);
    [[nodiscard]] std::optional<project_model::Enrollment> getEnrollment(
        int32_t studentId, int32_t subjectId) const;
    [[nodiscard]] bool updateGrade(int32_t studentId, int32_t subjectId, uint8_t grade);
    [[nodiscard]] bool unenroll(int32_t studentId, int32_t subjectId);
    [[nodiscard]] std::vector<project_model::Enrollment>
        getEnrollmentsByStudent(int32_t studentId) const;
    [[nodiscard]] std::vector<project_model::Enrollment>
        getEnrollmentsBySubject(int32_t subjectId) const;

    // ---- Auth ----
    [[nodiscard]] std::optional<LoginResult> login(
        const std::string& cpf, uint16_t password) const;

    // ---- Info ----
    [[nodiscard]] std::string getLastError() const { return lastError_; }
    [[nodiscard]] int32_t getNextStudentId() const;
    [[nodiscard]] int32_t getNextTeacherId() const;
    [[nodiscard]] int32_t getNextSubjectId() const;
    [[nodiscard]] int32_t getActiveCount(char type) const;

private:
    static constexpr int CHUNK_STU = 0;
    static constexpr int CHUNK_TCH = 1;
    static constexpr int CHUNK_SUB = 2;
    static constexpr int CHUNK_TREE = 3;

    std::fstream dataFile_;
    std::string dataDir_;
    mutable std::string lastError_;

    project_model::ChunkInfo chunks_[4];

    project_model::BPlusTree btree_;
    IndexCtrl indexCtrl_;

    bool readHeader();
    bool writeHeader();
    void computeChunkOffsets();

    int chunkIndexForType(char type) const;
    bool writeToChunk(int ci, size_t recIdx, const std::vector<std::byte>& data);
    std::vector<std::byte> readFromChunk(int ci, size_t recIdx) const;
    size_t appendToChunk(int ci, const std::vector<std::byte>& data);
    bool markDeletedInChunk(int ci, size_t recIdx);
    bool isDeletedInChunk(int ci, size_t recIdx) const;
    uint32_t countActiveInChunk(int ci) const;

    bool reallocateChunk(int ci);

    template<typename T>
    std::vector<T> scanAll(char type) const {
        int ci = chunkIndexForType(type);
        if (ci < 0) return {};

        std::vector<T> result;
        size_t total = static_cast<size_t>(chunks_[ci].used);
        result.reserve(total);

        for (size_t i = 0; i < total; ++i) {
            if (!isDeletedInChunk(ci, i)) {
                auto bytes = readFromChunk(ci, i);
                result.push_back(T::fromBytes(bytes));
            }
        }
        return result;
    }

    template<typename T>
    std::vector<T> scanAllWithIds(char type, std::vector<int32_t>& outIds) const {
        int ci = chunkIndexForType(type);
        if (ci < 0) return {};

        std::vector<T> result;
        size_t total = static_cast<size_t>(chunks_[ci].used);
        result.reserve(total);
        outIds.reserve(total);

        for (size_t i = 0; i < total; ++i) {
            if (!isDeletedInChunk(ci, i)) {
                auto bytes = readFromChunk(ci, i);
                outIds.push_back(static_cast<int32_t>(i));
                result.push_back(T::fromBytes(bytes));
            }
        }
        return result;
    }
};

} // namespace project_controller

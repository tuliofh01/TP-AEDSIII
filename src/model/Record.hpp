#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "../utility/Constants.hpp"
#include "../utility/Enums.hpp"

namespace project_model {

	// ========================================
	// Record Serializer (memcpy-based)
	// ========================================
	struct RecordSerializer {
		template<typename T>
		static void write(std::byte* buf, size_t off, const T& val) {
			std::memcpy(buf + off, &val, sizeof(T));
		}

		template<typename T>
		static T read(const std::byte* buf, size_t off) {
			T val;
			std::memcpy(&val, buf + off, sizeof(T));
			return val;
		}

		static void writeStr(std::byte* buf, size_t off, const char* str, size_t len) {
			std::memset(buf + off, 0, len);
			if (str) {
				size_t copyLen = std::strlen(str) < len ? std::strlen(str) : len - 1;
				std::memcpy(buf + off, str, copyLen);
			}
		}

		static std::string readStr(const std::byte* buf, size_t off, size_t len) {
			return std::string(reinterpret_cast<const char*>(buf + off), len).c_str();
		}
	};

	// ========================================
	// File Header (256 bytes at top of records.dat)
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
	// Chunk Info (32 bytes each)
	// ========================================
	struct ChunkInfo {
		char type = 0;
		char pad1[3]{};
		uint64_t offset = 0;
		uint32_t recordSize = 0;
		uint32_t capacity = project_utility::INITIAL_CHUNK_CAPACITY;
		uint32_t used = 0;
		uint32_t pad2 = 0;

		[[nodiscard]] std::vector<std::byte> toBytes() const;
		static ChunkInfo fromBytes(const std::vector<std::byte>& data);
	};

	// ========================================
	// UserRecord (C++ base — common fields)
	// ========================================
	struct UserRecord {
		using enum project_utility::RecStatus;

		char status = static_cast<char>(Ativo);
		char type = 0;
		int32_t userId = -1;
		char name[project_utility::NAME_LEN]{};
		char email[project_utility::EMAIL_LEN]{};
		char cpf[project_utility::CPF_LEN]{};

		[[nodiscard]] bool isActive() const {
			return status == static_cast<char>(Ativo);
		}

		void setActive() { status = static_cast<char>(Ativo); }
		void setDeleted() { status = static_cast<char>(Deletado); }

		void writeCommonFields(std::byte* buf) const;
		void readCommonFields(const std::byte* buf);

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

	// ========================================
	// StudentRecord (type 'S', 139 bytes)
	// ========================================
	struct StudentRecord : public UserRecord {
		uint32_t birthDate = 0;
		char courseName[project_utility::COURSE_LEN]{};
		int32_t enrollmentYear = 0;

		[[nodiscard]] std::vector<std::byte> toBytes() const;
		[[nodiscard]] static StudentRecord fromBytes(const std::vector<std::byte>& data);

		[[nodiscard]] std::string courseStr() const {
			return std::string(courseName, project_utility::COURSE_LEN).c_str();
		}
	};

	// ========================================
	// TeacherRecord (type 'T', 165 bytes)
	// ========================================
	struct TeacherRecord : public UserRecord {
		char department[project_utility::DEPT_LEN]{};
		char specialization[project_utility::SPEC_LEN]{};
		uint32_t hireDate = 0;

		[[nodiscard]] std::vector<std::byte> toBytes() const;
		[[nodiscard]] static TeacherRecord fromBytes(const std::vector<std::byte>& data);

		[[nodiscard]] std::string deptStr() const {
			return std::string(department, project_utility::DEPT_LEN).c_str();
		}

		[[nodiscard]] std::string specStr() const {
			return std::string(specialization, project_utility::SPEC_LEN).c_str();
		}
	};

	// ========================================
	// SubjectRecord (type 'B', 84 bytes)
	// ========================================
	struct SubjectRecord {
		using enum project_utility::RecStatus;

		char status = static_cast<char>(Ativo);
		char type = static_cast<char>(project_utility::RecType::Subject);
		int32_t subjectId = -1;
		char name[project_utility::NAME_LEN]{};
		char code[project_utility::SUBJ_CODE_LEN]{};
		int32_t credits = 0;
		int32_t teacherId = -1;

		[[nodiscard]] bool isActive() const {
			return status == static_cast<char>(Ativo);
		}

		void setActive() { status = static_cast<char>(Ativo); }
		void setDeleted() { status = static_cast<char>(Deletado); }

		[[nodiscard]] std::vector<std::byte> toBytes() const;
		[[nodiscard]] static SubjectRecord fromBytes(const std::vector<std::byte>& data);

		[[nodiscard]] std::string nameStr() const {
			return std::string(name, project_utility::NAME_LEN).c_str();
		}

		[[nodiscard]] std::string codeStr() const {
			return std::string(code, project_utility::SUBJ_CODE_LEN).c_str();
		}
	};

	// ========================================
	// B+ Tree Leaf Value (enrollment data)
	// ========================================
	struct BTreeLeafValue {
		int32_t studentId = -1;
		int32_t subjectId = -1;
		int32_t teacherId = -1;
		float grade = 0.0f;
		char semester[project_utility::SEMESTER_LEN]{};

		[[nodiscard]] std::vector<std::byte> toBytes() const;
		[[nodiscard]] static BTreeLeafValue fromBytes(const std::vector<std::byte>& data);

		[[nodiscard]] std::string semesterStr() const {
			return std::string(semester, project_utility::SEMESTER_LEN).c_str();
		}
	};

} // namespace project_model

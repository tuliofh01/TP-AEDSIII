#include "Record.hpp"
#include <cstring>

namespace project_model {

	// ========================================
	// FileHeader
	// ========================================
	auto FileHeader::toBytes() const -> std::vector<std::byte> {
		using namespace project_utility;
		std::vector<std::byte> bytes(FILE_HEADER_SIZE);
		RecordSerializer::write(bytes.data(), 0, magic);
		RecordSerializer::write(bytes.data(), 4, version);
		RecordSerializer::write(bytes.data(), 8, headerSize);
		RecordSerializer::write(bytes.data(), 12, chunkCount);
		RecordSerializer::write(bytes.data(), 16, nextStudentId);
		RecordSerializer::write(bytes.data(), 20, nextTeacherId);
		RecordSerializer::write(bytes.data(), 24, nextSubjectId);
		RecordSerializer::write(bytes.data(), 28, padding1);
		std::memcpy(bytes.data() + 32, reserved, FILE_HEADER_SIZE - 32);
		return bytes;
	}

	auto FileHeader::fromBytes(const std::vector<std::byte>& data) -> FileHeader {
		using namespace project_utility;
		FileHeader hdr;
		if (data.size() >= FILE_HEADER_SIZE) {
			auto* buf = data.data();
			hdr.magic = RecordSerializer::read<uint32_t>(buf, 0);
			hdr.version = RecordSerializer::read<uint32_t>(buf, 4);
			hdr.headerSize = RecordSerializer::read<uint32_t>(buf, 8);
			hdr.chunkCount = RecordSerializer::read<uint32_t>(buf, 12);
			hdr.nextStudentId = RecordSerializer::read<uint32_t>(buf, 16);
			hdr.nextTeacherId = RecordSerializer::read<uint32_t>(buf, 20);
			hdr.nextSubjectId = RecordSerializer::read<uint32_t>(buf, 24);
			hdr.padding1 = RecordSerializer::read<uint32_t>(buf, 28);
			std::memcpy(hdr.reserved, buf + 32, FILE_HEADER_SIZE - 32);
		}
		return hdr;
	}

	// ========================================
	// ChunkInfo
	// ========================================
	auto ChunkInfo::toBytes() const -> std::vector<std::byte> {
		using namespace project_utility;
		std::vector<std::byte> bytes(CHUNK_INFO_SIZE);
		RecordSerializer::write(bytes.data(), 0, type);
		RecordSerializer::write(bytes.data(), 1, pad1[0]);
		RecordSerializer::write(bytes.data(), 2, pad1[1]);
		RecordSerializer::write(bytes.data(), 3, pad1[2]);
		RecordSerializer::write(bytes.data(), 4, offset);
		RecordSerializer::write(bytes.data(), 12, recordSize);
		RecordSerializer::write(bytes.data(), 16, capacity);
		RecordSerializer::write(bytes.data(), 20, used);
		RecordSerializer::write(bytes.data(), 24, pad2);
		return bytes;
	}

	auto ChunkInfo::fromBytes(const std::vector<std::byte>& data) -> ChunkInfo {
		using namespace project_utility;
		ChunkInfo ci;
		if (data.size() >= CHUNK_INFO_SIZE) {
			auto* buf = data.data();
			ci.type = RecordSerializer::read<char>(buf, 0);
			ci.pad1[0] = RecordSerializer::read<char>(buf, 1);
			ci.pad1[1] = RecordSerializer::read<char>(buf, 2);
			ci.pad1[2] = RecordSerializer::read<char>(buf, 3);
			ci.offset = RecordSerializer::read<uint64_t>(buf, 4);
			ci.recordSize = RecordSerializer::read<uint32_t>(buf, 12);
			ci.capacity = RecordSerializer::read<uint32_t>(buf, 16);
			ci.used = RecordSerializer::read<uint32_t>(buf, 20);
			ci.pad2 = RecordSerializer::read<uint32_t>(buf, 24);
		}
		return ci;
	}

	// ========================================
	// UserRecord common fields
	// ========================================
	void UserRecord::writeCommonFields(std::byte* buf) const {
		using namespace project_utility;
		RecordSerializer::write<char>(buf, 0, status);
		RecordSerializer::write<char>(buf, OFFSET_REC_TYPE, type);
		RecordSerializer::write(buf, OFFSET_USER_ID2, userId);
		RecordSerializer::writeStr(buf, OFFSET_NAME2, name, NAME_LEN);
		RecordSerializer::writeStr(buf, OFFSET_EMAIL, email, EMAIL_LEN);
		RecordSerializer::writeStr(buf, OFFSET_CPF, cpf, CPF_LEN);
	}

	void UserRecord::readCommonFields(const std::byte* buf) {
		using namespace project_utility;
		status = RecordSerializer::read<char>(buf, 0);
		type = RecordSerializer::read<char>(buf, OFFSET_REC_TYPE);
		userId = RecordSerializer::read<int32_t>(buf, OFFSET_USER_ID2);
		auto s = RecordSerializer::readStr(buf, OFFSET_NAME2, NAME_LEN);
		std::strncpy(name, s.c_str(), NAME_LEN - 1);
		name[NAME_LEN - 1] = '\0';
		s = RecordSerializer::readStr(buf, OFFSET_EMAIL, EMAIL_LEN);
		std::strncpy(email, s.c_str(), EMAIL_LEN - 1);
		email[EMAIL_LEN - 1] = '\0';
		s = RecordSerializer::readStr(buf, OFFSET_CPF, CPF_LEN);
		std::strncpy(cpf, s.c_str(), CPF_LEN - 1);
		cpf[CPF_LEN - 1] = '\0';
	}

	// ========================================
	// StudentRecord (139 bytes)
	// ========================================
	auto StudentRecord::toBytes() const -> std::vector<std::byte> {
		using namespace project_utility;
		std::vector<std::byte> bytes(STUDENT_RECORD_SIZE);
		writeCommonFields(bytes.data());
		RecordSerializer::write(bytes.data(), OFFSET_BIRTH_DATE2, birthDate);
		RecordSerializer::writeStr(bytes.data(), OFFSET_COURSE, courseName, COURSE_LEN);
		RecordSerializer::write(bytes.data(), OFFSET_ENROLL_YEAR, enrollmentYear);
		return bytes;
	}

	auto StudentRecord::fromBytes(const std::vector<std::byte>& data) -> StudentRecord {
		using namespace project_utility;
		StudentRecord rec;
		if (data.size() >= STUDENT_RECORD_SIZE) {
			auto* buf = data.data();
			rec.readCommonFields(buf);
			rec.birthDate = RecordSerializer::read<uint32_t>(buf, OFFSET_BIRTH_DATE2);
			auto s = RecordSerializer::readStr(buf, OFFSET_COURSE, COURSE_LEN);
			std::strncpy(rec.courseName, s.c_str(), COURSE_LEN - 1);
			rec.courseName[COURSE_LEN - 1] = '\0';
			rec.enrollmentYear = RecordSerializer::read<int32_t>(buf, OFFSET_ENROLL_YEAR);
		}
		return rec;
	}

	// ========================================
	// TeacherRecord (165 bytes)
	// ========================================
	auto TeacherRecord::toBytes() const -> std::vector<std::byte> {
		using namespace project_utility;
		std::vector<std::byte> bytes(TEACHER_RECORD_SIZE);
		writeCommonFields(bytes.data());
		RecordSerializer::writeStr(bytes.data(), OFFSET_DEPT, department, DEPT_LEN);
		RecordSerializer::writeStr(bytes.data(), OFFSET_SPEC, specialization, SPEC_LEN);
		RecordSerializer::write(bytes.data(), OFFSET_HIRE_DATE, hireDate);
		return bytes;
	}

	auto TeacherRecord::fromBytes(const std::vector<std::byte>& data) -> TeacherRecord {
		using namespace project_utility;
		TeacherRecord rec;
		if (data.size() >= TEACHER_RECORD_SIZE) {
			auto* buf = data.data();
			rec.readCommonFields(buf);
			auto s = RecordSerializer::readStr(buf, OFFSET_DEPT, DEPT_LEN);
			std::strncpy(rec.department, s.c_str(), DEPT_LEN - 1);
			rec.department[DEPT_LEN - 1] = '\0';
			s = RecordSerializer::readStr(buf, OFFSET_SPEC, SPEC_LEN);
			std::strncpy(rec.specialization, s.c_str(), SPEC_LEN - 1);
			rec.specialization[SPEC_LEN - 1] = '\0';
			rec.hireDate = RecordSerializer::read<uint32_t>(buf, OFFSET_HIRE_DATE);
		}
		return rec;
	}

	// ========================================
	// SubjectRecord (84 bytes)
	// ========================================
	auto SubjectRecord::toBytes() const -> std::vector<std::byte> {
		using namespace project_utility;
		std::vector<std::byte> bytes(SUBJECT_RECORD_SIZE);
		RecordSerializer::write<char>(bytes.data(), 0, status);
		RecordSerializer::write<char>(bytes.data(), SUBJ_OFFSET_TYPE, type);
		RecordSerializer::write(bytes.data(), SUBJ_OFFSET_ID, subjectId);
		RecordSerializer::writeStr(bytes.data(), SUBJ_OFFSET_NAME, name, NAME_LEN);
		RecordSerializer::writeStr(bytes.data(), SUBJ_OFFSET_CODE, code, SUBJ_CODE_LEN);
		RecordSerializer::write(bytes.data(), SUBJ_OFFSET_CREDITS, credits);
		RecordSerializer::write(bytes.data(), SUBJ_OFFSET_TEACHER, teacherId);
		return bytes;
	}

	auto SubjectRecord::fromBytes(const std::vector<std::byte>& data) -> SubjectRecord {
		using namespace project_utility;
		SubjectRecord rec;
		if (data.size() >= SUBJECT_RECORD_SIZE) {
			auto* buf = data.data();
			rec.status = RecordSerializer::read<char>(buf, 0);
			rec.type = RecordSerializer::read<char>(buf, SUBJ_OFFSET_TYPE);
			rec.subjectId = RecordSerializer::read<int32_t>(buf, SUBJ_OFFSET_ID);
			auto s = RecordSerializer::readStr(buf, SUBJ_OFFSET_NAME, NAME_LEN);
			std::strncpy(rec.name, s.c_str(), NAME_LEN - 1);
			rec.name[NAME_LEN - 1] = '\0';
			s = RecordSerializer::readStr(buf, SUBJ_OFFSET_CODE, SUBJ_CODE_LEN);
			std::strncpy(rec.code, s.c_str(), SUBJ_CODE_LEN - 1);
			rec.code[SUBJ_CODE_LEN - 1] = '\0';
			rec.credits = RecordSerializer::read<int32_t>(buf, SUBJ_OFFSET_CREDITS);
			rec.teacherId = RecordSerializer::read<int32_t>(buf, SUBJ_OFFSET_TEACHER);
		}
		return rec;
	}

	// ========================================
	// BTreeLeafValue (enrollment data)
	// ========================================
	auto BTreeLeafValue::toBytes() const -> std::vector<std::byte> {
		using namespace project_utility;
		std::vector<std::byte> bytes(BTREE_LEAF_VALUE_SIZE);
		RecordSerializer::write(bytes.data(), 0, studentId);
		RecordSerializer::write(bytes.data(), 4, subjectId);
		RecordSerializer::write(bytes.data(), 8, teacherId);
		RecordSerializer::write(bytes.data(), 12, grade);
		RecordSerializer::writeStr(bytes.data(), 16, semester, SEMESTER_LEN);
		return bytes;
	}

	auto BTreeLeafValue::fromBytes(const std::vector<std::byte>& data) -> BTreeLeafValue {
		using namespace project_utility;
		BTreeLeafValue val;
		if (data.size() >= BTREE_LEAF_VALUE_SIZE) {
			auto* buf = data.data();
			val.studentId = RecordSerializer::read<int32_t>(buf, 0);
			val.subjectId = RecordSerializer::read<int32_t>(buf, 4);
			val.teacherId = RecordSerializer::read<int32_t>(buf, 8);
			val.grade = RecordSerializer::read<float>(buf, 12);
			auto s = RecordSerializer::readStr(buf, 16, SEMESTER_LEN);
			std::strncpy(val.semester, s.c_str(), SEMESTER_LEN - 1);
			val.semester[SEMESTER_LEN - 1] = '\0';
		}
		return val;
	}

} // namespace project_model

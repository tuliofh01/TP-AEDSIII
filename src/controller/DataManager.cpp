#include "DataManager.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>

namespace project_controller {

	// ========================================
	// Destructor — ensure file is flushed
	// ========================================
	DataManager::~DataManager() {
		if (dataFile_.is_open()) {
			dataFile_.flush();
			dataFile_.close();
		}
	}

	// ========================================
	// Initialize — open/create records.dat
	// ========================================
	auto DataManager::initialize(const std::string& dataDir) -> bool {
		using namespace project_utility;
		using namespace project_model;

		dataDir_ = dataDir;
		auto dataPath = dataDir + "/" + std::string(DATA_FILE);
		auto idxPath = dataDir + "/" + std::string(INDEX_FILE);

		// Ensure data directory exists
		std::filesystem::create_directories(dataDir);

		bool fileExists = std::filesystem::exists(dataPath);

		// Open the data file
		dataFile_.open(dataPath,
			std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
		if (!dataFile_.is_open()) {
			dataFile_.open(dataPath,
				std::ios::binary | std::ios::out | std::ios::trunc);
			if (!dataFile_.is_open()) {
				lastError_ = "Falha ao criar records.dat";
				return false;
			}
			fileExists = false;
		}

		if (!fileExists) {
			// Initialize chunks with defaults
			chunks_[CHUNK_STU] = ChunkInfo{};
			chunks_[CHUNK_STU].type = CHUNK_STUDENT;
			chunks_[CHUNK_STU].recordSize = STUDENT_RECORD_SIZE;
			chunks_[CHUNK_STU].capacity = INITIAL_CHUNK_CAPACITY;
			chunks_[CHUNK_STU].used = 0;

			chunks_[CHUNK_TCH] = ChunkInfo{};
			chunks_[CHUNK_TCH].type = CHUNK_TEACHER;
			chunks_[CHUNK_TCH].recordSize = TEACHER_RECORD_SIZE;
			chunks_[CHUNK_TCH].capacity = INITIAL_CHUNK_CAPACITY;
			chunks_[CHUNK_TCH].used = 0;

			chunks_[CHUNK_SUB] = ChunkInfo{};
			chunks_[CHUNK_SUB].type = CHUNK_SUBJECT;
			chunks_[CHUNK_SUB].recordSize = SUBJECT_RECORD_SIZE;
			chunks_[CHUNK_SUB].capacity = INITIAL_CHUNK_CAPACITY;
			chunks_[CHUNK_SUB].used = 0;

			chunks_[CHUNK_TREE] = ChunkInfo{};
			chunks_[CHUNK_TREE].type = CHUNK_TREE;
			chunks_[CHUNK_TREE].recordSize = BTREE_PAGE_SIZE;
			chunks_[CHUNK_TREE].capacity = 4;  // 4 initial pages
			chunks_[CHUNK_TREE].used = 0;

			computeChunkOffsets();
			writeHeader();

			// Fill tree chunk with zeroed pages
			size_t treeStart = chunks_[CHUNK_TREE].offset + BPlusTree::CHUNK_HEADER;
			for (uint32_t i = 0; i < chunks_[CHUNK_TREE].capacity; ++i) {
				std::vector<std::byte> page(BTREE_PAGE_SIZE, std::byte{0});
				dataFile_.seekp(static_cast<std::streamoff>(treeStart + i * BTREE_PAGE_SIZE));
				dataFile_.write(reinterpret_cast<const char*>(page.data()), BTREE_PAGE_SIZE);
			}
			chunks_[CHUNK_TREE].used = chunks_[CHUNK_TREE].capacity;
		} else {
			// Read existing header
			if (!readHeader()) {
				lastError_ = "Falha ao ler cabecalho";
				return false;
			}
			computeChunkOffsets();
		}

		// Initialize B+ tree
		if (!btree_.initialize(dataFile_, chunks_[CHUNK_TREE].offset,
				chunks_[CHUNK_TREE].used > 0 ? chunks_[CHUNK_TREE].used : 4)) {
			lastError_ = "Falha ao inicializar B+ Tree";
			return false;
		}

		// Initialize hash index
		if (!indexCtrl_.initialize(idxPath)) {
			lastError_ = "Falha ao inicializar indice hash";
			return false;
		}

		return true;
	}

	// ========================================
	// Header I/O
	// ========================================
	auto DataManager::readHeader() -> bool {
		using namespace project_model;
		FileHeader hdr;
		std::vector<std::byte> hdrBytes(project_utility::FILE_HEADER_SIZE);
		dataFile_.seekg(0, std::ios::beg);
		dataFile_.read(reinterpret_cast<char*>(hdrBytes.data()), hdrBytes.size());
		if (!dataFile_.good() && !dataFile_.eof()) return false;
		hdr = FileHeader::fromBytes(hdrBytes);

		// Read chunk table (embedded in header reserved area at offset 32)
		auto* buf = hdrBytes.data();
		for (uint32_t i = 0; i < 4 && i < hdr.chunkCount; ++i) {
			std::vector<std::byte> ciBytes(buf + 32 + i * project_utility::CHUNK_INFO_SIZE,
				buf + 32 + (i + 1) * project_utility::CHUNK_INFO_SIZE);
			chunks_[i] = ChunkInfo::fromBytes(ciBytes);
		}
		return true;
	}

	auto DataManager::writeHeader() -> bool {
		using namespace project_model;
		using namespace project_utility;

		FileHeader hdr;
		// IDs come from header
		// Preserve existing counters if available — we'll reconstruct from index
		auto hdrBytes = hdr.toBytes();

		// Embed chunk infos at offset 32
		for (uint32_t i = 0; i < CHUNK_COUNT; ++i) {
			auto ciBytes = chunks_[i].toBytes();
			std::memcpy(hdrBytes.data() + 32 + i * CHUNK_INFO_SIZE, ciBytes.data(), CHUNK_INFO_SIZE);
		}

		dataFile_.seekp(0, std::ios::beg);
		dataFile_.write(reinterpret_cast<const char*>(hdrBytes.data()), hdrBytes.size());
		dataFile_.flush();
		return dataFile_.good();
	}

	void DataManager::computeChunkOffsets() {
		size_t off = project_utility::FILE_HEADER_SIZE;
		for (int i = 0; i < 4; ++i) {
			chunks_[i].offset = off;
			off += static_cast<size_t>(chunks_[i].capacity) * chunks_[i].recordSize;
		}
	}

	// ========================================
	// Chunk helpers
	// ========================================
	int DataManager::chunkIndexForType(char type) const {
		for (int i = 0; i < 4; ++i)
			if (chunks_[i].type == type) return i;
		return -1;
	}

	bool DataManager::writeToChunk(int ci, size_t recIdx, const std::vector<std::byte>& data) {
		if (recIdx >= static_cast<size_t>(chunks_[ci].capacity)) return false;
		size_t off = chunks_[ci].offset + recIdx * chunks_[ci].recordSize;
		dataFile_.seekp(static_cast<std::streamoff>(off), std::ios::beg);
		dataFile_.write(reinterpret_cast<const char*>(data.data()), chunks_[ci].recordSize);
		return dataFile_.good();
	}

	std::vector<std::byte> DataManager::readFromChunk(int ci, size_t recIdx) const {
		std::vector<std::byte> buf(chunks_[ci].recordSize);
		size_t off = chunks_[ci].offset + recIdx * chunks_[ci].recordSize;
		auto& f = const_cast<std::fstream&>(dataFile_);
		f.seekg(static_cast<std::streamoff>(off), std::ios::beg);
		f.read(reinterpret_cast<char*>(buf.data()), chunks_[ci].recordSize);
		return buf;
	}

	size_t DataManager::appendToChunk(int ci, const std::vector<std::byte>& data) {
		if (chunks_[ci].used >= chunks_[ci].capacity) {
			if (!const_cast<DataManager*>(this)->reallocateChunk(ci))
				return static_cast<size_t>(-1);
		}

		size_t idx = chunks_[ci].used;
		if (writeToChunk(ci, idx, data)) {
			chunks_[ci].used++;
			writeHeader();
			return idx;
		}
		return static_cast<size_t>(-1);
	}

	bool DataManager::markDeletedInChunk(int ci, size_t recIdx) {
		if (recIdx >= static_cast<size_t>(chunks_[ci].used)) return false;
		size_t off = chunks_[ci].offset + recIdx * chunks_[ci].recordSize;
		char deleted = static_cast<char>(project_utility::RecStatus::Deletado);
		dataFile_.seekp(static_cast<std::streamoff>(off), std::ios::beg);
		dataFile_.write(&deleted, 1);
		return dataFile_.good();
	}

	bool DataManager::isDeletedInChunk(int ci, size_t recIdx) const {
		if (recIdx >= static_cast<size_t>(chunks_[ci].used)) return true;
		size_t off = chunks_[ci].offset + recIdx * chunks_[ci].recordSize;
		char status = 0;
		auto& f = const_cast<std::fstream&>(dataFile_);
		f.seekg(static_cast<std::streamoff>(off), std::ios::beg);
		f.read(&status, 1);
		return status == static_cast<char>(project_utility::RecStatus::Deletado);
	}

	uint32_t DataManager::countActiveInChunk(int ci) const {
		uint32_t count = 0;
		for (size_t i = 0; i < static_cast<size_t>(chunks_[ci].used); ++i) {
			if (!isDeletedInChunk(ci, i)) ++count;
		}
		return count;
	}

	// ========================================
	// File recreation
	// ========================================
	bool DataManager::reallocateChunk(int ci) {
		using namespace project_utility;

		// Save current chunk capacities
		std::array<uint32_t, 4> oldCaps;
		oldCaps[0] = chunks_[0].capacity;
		oldCaps[1] = chunks_[1].capacity;
		oldCaps[2] = chunks_[2].capacity;
		oldCaps[3] = chunks_[3].capacity;

		// Double the overflowing chunk's capacity
		chunks_[ci].capacity *= 2;

		// Compute new offsets
		size_t oldTreeOffset = chunks_[CHUNK_TREE].offset;
		computeChunkOffsets();

		// Write updated header
		writeHeader();

		// Calculate total new file size
		size_t newFileSize = chunks_[CHUNK_TREE].offset +
			static_cast<size_t>(chunks_[CHUNK_TREE].capacity) * BTREE_PAGE_SIZE;

		// Set file to new size (extends file)
		dataFile_.seekp(0, std::ios::end);
		auto currentSize = dataFile_.tellp();
		if (static_cast<size_t>(currentSize) < newFileSize) {
			// Write a zero byte at the new end to extend
			dataFile_.seekp(static_cast<std::streamoff>(newFileSize - 1));
			char zero = 0;
			dataFile_.write(&zero, 1);
		}

		// Move data chunks that shifted. We need to move from right to left
		// to avoid overwriting. Actually, since the overflowing chunk may be
		// in the middle, later chunks need to be shifted right.
		// The safest approach: rewrite the entire file.

		// Read all existing data into memory
		struct ChunkData {
			std::vector<std::byte> data;
			size_t oldOffset;
			size_t newOffset;
		};
		std::vector<ChunkData> chunks;

		size_t oldOff = FILE_HEADER_SIZE;
		for (int i = 0; i < 4; ++i) {
			size_t chunkBytes = static_cast<size_t>(oldCaps[i]) * chunks_[i].recordSize;
			if (chunkBytes > 0) {
				ChunkData cd;
				cd.data.resize(chunkBytes);
				cd.oldOffset = oldOff;
				cd.newOffset = chunks_[i].offset;
				dataFile_.seekg(static_cast<std::streamoff>(oldOff), std::ios::beg);
				dataFile_.read(reinterpret_cast<char*>(cd.data.data()), chunkBytes);
				chunks.push_back(std::move(cd));
			}
			oldOff += chunkBytes;
		}

		// Write all chunks back at new offsets (last first to avoid overlap)
		std::sort(chunks.begin(), chunks.end(),
			[](const ChunkData& a, const ChunkData& b) {
				return a.newOffset > b.newOffset; // write last first
			});

		for (const auto& cd : chunks) {
			dataFile_.seekp(static_cast<std::streamoff>(cd.newOffset), std::ios::beg);
			dataFile_.write(reinterpret_cast<const char*>(cd.data.data()), cd.data.size());
		}

		// Update B+ tree chunk offset
		if (ci != CHUNK_TREE) {
			// Tree chunk moved — reinitialize B+ tree with new offset
			btree_.initialize(dataFile_, chunks_[CHUNK_TREE].offset,
				chunks_[CHUNK_TREE].used);
		}

		dataFile_.flush();
		return true;
	}

	// ========================================
	// Student CRUD
	// ========================================
	auto DataManager::createStudent(
		const std::string& name, const std::string& email,
		const std::string& cpf, uint32_t birthDate,
		const std::string& courseName, int32_t enrollmentYear) -> bool
	{
		using namespace project_model;

		if (name.empty()) {
			lastError_ = "Nome nao pode ser vazio";
			return false;
		}

		StudentRecord rec;
		rec.type = project_utility::CHUNK_STUDENT;
		std::strncpy(rec.name, name.c_str(), project_utility::NAME_LEN - 1);
		rec.name[project_utility::NAME_LEN - 1] = '\0';
		std::strncpy(rec.email, email.c_str(), project_utility::EMAIL_LEN - 1);
		rec.email[project_utility::EMAIL_LEN - 1] = '\0';
		std::strncpy(rec.cpf, cpf.c_str(), project_utility::CPF_LEN - 1);
		rec.cpf[project_utility::CPF_LEN - 1] = '\0';
		rec.birthDate = birthDate;
		std::strncpy(rec.courseName, courseName.c_str(), project_utility::COURSE_LEN - 1);
		rec.courseName[project_utility::COURSE_LEN - 1] = '\0';
		rec.enrollmentYear = enrollmentYear;

		// Assign ID from header
		auto id = indexCtrl_.nextId("STU:");
		rec.userId = static_cast<int32_t>(id);

		auto bytes = rec.toBytes();
		size_t idx = appendToChunk(CHUNK_STU, bytes);
		if (idx == static_cast<size_t>(-1)) {
			lastError_ = "Erro ao escrever estudante";
			return false;
		}

		// Update index
		indexCtrl_.insert("STU:" + std::to_string(rec.userId), CHUNK_STU, static_cast<uint32_t>(idx));
		indexCtrl_.insert("NM:" + name, CHUNK_STU, static_cast<uint32_t>(idx));
		indexCtrl_.save();

		return true;
	}

	auto DataManager::readStudent(int32_t id) const -> std::optional<project_model::StudentRecord> {
		auto idxOpt = indexCtrl_.lookup("STU:" + std::to_string(id));
		if (!idxOpt) {
			lastError_ = "Estudante nao encontrado: " + std::to_string(id);
			return std::nullopt;
		}

		int ci = CHUNK_STU;
		auto bytes = readFromChunk(ci, idxOpt->recordIndex);
		auto rec = project_model::StudentRecord::fromBytes(bytes);
		if (!rec.isActive()) {
			lastError_ = "Registro deletado";
			return std::nullopt;
		}
		return rec;
	}

	auto DataManager::deleteStudent(int32_t id) -> bool {
		auto idxOpt = indexCtrl_.lookup("STU:" + std::to_string(id));
		if (!idxOpt) {
			lastError_ = "Estudante nao encontrado: " + std::to_string(id);
			return false;
		}

		if (markDeletedInChunk(CHUNK_STU, idxOpt->recordIndex)) {
			indexCtrl_.remove("STU:" + std::to_string(id));
			(void)indexCtrl_.save();
			return true;
		}

		lastError_ = "Erro ao deletar estudante";
		return false;
	}

	auto DataManager::listAllStudents() const -> std::vector<project_model::StudentRecord> {
		return scanAll<project_model::StudentRecord>(project_utility::CHUNK_STUDENT);
	}

	// ========================================
	// Teacher CRUD
	// ========================================
	auto DataManager::createTeacher(
		const std::string& name, const std::string& email,
		const std::string& cpf, const std::string& department,
		const std::string& specialization, uint32_t hireDate) -> bool
	{
		using namespace project_model;

		if (name.empty()) {
			lastError_ = "Nome nao pode ser vazio";
			return false;
		}

		TeacherRecord rec;
		rec.type = project_utility::CHUNK_TEACHER;
		std::strncpy(rec.name, name.c_str(), project_utility::NAME_LEN - 1);
		rec.name[project_utility::NAME_LEN - 1] = '\0';
		std::strncpy(rec.email, email.c_str(), project_utility::EMAIL_LEN - 1);
		rec.email[project_utility::EMAIL_LEN - 1] = '\0';
		std::strncpy(rec.cpf, cpf.c_str(), project_utility::CPF_LEN - 1);
		rec.cpf[project_utility::CPF_LEN - 1] = '\0';
		std::strncpy(rec.department, department.c_str(), project_utility::DEPT_LEN - 1);
		rec.department[project_utility::DEPT_LEN - 1] = '\0';
		std::strncpy(rec.specialization, specialization.c_str(), project_utility::SPEC_LEN - 1);
		rec.specialization[project_utility::SPEC_LEN - 1] = '\0';
		rec.hireDate = hireDate;

		auto id = indexCtrl_.nextId("TCH:");
		rec.userId = static_cast<int32_t>(id);

		auto bytes = rec.toBytes();
		size_t idx = appendToChunk(CHUNK_TCH, bytes);
		if (idx == static_cast<size_t>(-1)) {
			lastError_ = "Erro ao escrever professor";
			return false;
		}

		indexCtrl_.insert("TCH:" + std::to_string(rec.userId), CHUNK_TCH, static_cast<uint32_t>(idx));
		indexCtrl_.insert("NM:" + name, CHUNK_TCH, static_cast<uint32_t>(idx));
		indexCtrl_.save();

		return true;
	}

	auto DataManager::readTeacher(int32_t id) const -> std::optional<project_model::TeacherRecord> {
		auto idxOpt = indexCtrl_.lookup("TCH:" + std::to_string(id));
		if (!idxOpt) {
			lastError_ = "Professor nao encontrado: " + std::to_string(id);
			return std::nullopt;
		}

		auto bytes = readFromChunk(CHUNK_TCH, idxOpt->recordIndex);
		auto rec = project_model::TeacherRecord::fromBytes(bytes);
		if (!rec.isActive()) {
			lastError_ = "Registro deletado";
			return std::nullopt;
		}
		return rec;
	}

	auto DataManager::deleteTeacher(int32_t id) -> bool {
		auto idxOpt = indexCtrl_.lookup("TCH:" + std::to_string(id));
		if (!idxOpt) {
			lastError_ = "Professor nao encontrado: " + std::to_string(id);
			return false;
		}

		if (markDeletedInChunk(CHUNK_TCH, idxOpt->recordIndex)) {
			indexCtrl_.remove("TCH:" + std::to_string(id));
			(void)indexCtrl_.save();
			return true;
		}

		lastError_ = "Erro ao deletar professor";
		return false;
	}

	auto DataManager::listAllTeachers() const -> std::vector<project_model::TeacherRecord> {
		return scanAll<project_model::TeacherRecord>(project_utility::CHUNK_TEACHER);
	}

	// ========================================
	// Subject CRUD
	// ========================================
	auto DataManager::createSubject(
		const std::string& name, const std::string& code,
		int32_t credits, int32_t teacherId) -> bool
	{
		using namespace project_model;

		if (name.empty()) {
			lastError_ = "Nome nao pode ser vazio";
			return false;
		}

		SubjectRecord rec;
		std::strncpy(rec.name, name.c_str(), project_utility::NAME_LEN - 1);
		rec.name[project_utility::NAME_LEN - 1] = '\0';
		std::strncpy(rec.code, code.c_str(), project_utility::SUBJ_CODE_LEN - 1);
		rec.code[project_utility::SUBJ_CODE_LEN - 1] = '\0';
		rec.credits = credits;
		rec.teacherId = teacherId;

		auto id = indexCtrl_.nextId("SUB:");
		rec.subjectId = static_cast<int32_t>(id);

		auto bytes = rec.toBytes();
		size_t idx = appendToChunk(CHUNK_SUB, bytes);
		if (idx == static_cast<size_t>(-1)) {
			lastError_ = "Erro ao escrever disciplina";
			return false;
		}

		indexCtrl_.insert("SUB:" + std::to_string(rec.subjectId), CHUNK_SUB, static_cast<uint32_t>(idx));
		indexCtrl_.insert("NM:" + name, CHUNK_SUB, static_cast<uint32_t>(idx));
		indexCtrl_.save();

		return true;
	}

	auto DataManager::readSubject(int32_t id) const -> std::optional<project_model::SubjectRecord> {
		auto idxOpt = indexCtrl_.lookup("SUB:" + std::to_string(id));
		if (!idxOpt) {
			lastError_ = "Disciplina nao encontrada: " + std::to_string(id);
			return std::nullopt;
		}

		auto bytes = readFromChunk(CHUNK_SUB, idxOpt->recordIndex);
		auto rec = project_model::SubjectRecord::fromBytes(bytes);
		if (!rec.isActive()) {
			lastError_ = "Registro deletado";
			return std::nullopt;
		}
		return rec;
	}

	auto DataManager::deleteSubject(int32_t id) -> bool {
		auto idxOpt = indexCtrl_.lookup("SUB:" + std::to_string(id));
		if (!idxOpt) {
			lastError_ = "Disciplina nao encontrada: " + std::to_string(id);
			return false;
		}

		if (markDeletedInChunk(CHUNK_SUB, idxOpt->recordIndex)) {
			indexCtrl_.remove("SUB:" + std::to_string(id));
			(void)indexCtrl_.save();
			return true;
		}

		lastError_ = "Erro ao deletar disciplina";
		return false;
	}

	auto DataManager::listAllSubjects() const -> std::vector<project_model::SubjectRecord> {
		return scanAll<project_model::SubjectRecord>(project_utility::CHUNK_SUBJECT);
	}

	// ========================================
	// Enrollment (via B+ Tree)
	// ========================================
	static std::string padId(int32_t id) {
		auto s = std::to_string(id);
		if (s.size() < 4) s = std::string(4 - s.size(), '0') + s;
		return s;
	}

	static std::string enrollmentKey(int32_t studentId, int32_t subjectId) {
		return "ENR:STU:" + padId(studentId) + ":SUB:" + padId(subjectId);
	}

	static std::string enrollmentsByStudentPrefix(int32_t studentId) {
		return "ENR:STU:" + padId(studentId) + ":";
	}

	auto DataManager::enrollStudent(
		int32_t studentId, int32_t subjectId,
		int32_t teacherId, const std::string& semester) -> bool
	{
		using namespace project_model;

		auto key = enrollmentKey(studentId, subjectId);

		// Check if already enrolled
		if (btree_.search(key).has_value()) {
			lastError_ = "Estudante ja matriculado nesta disciplina";
			return false;
		}

		BTreeLeafValue val;
		val.studentId = studentId;
		val.subjectId = subjectId;
		val.teacherId = teacherId;
		val.grade = 0.0f;
		std::strncpy(val.semester, semester.c_str(), project_utility::SEMESTER_LEN - 1);
		val.semester[project_utility::SEMESTER_LEN - 1] = '\0';

		if (!btree_.insert(key, val)) {
			lastError_ = "Erro ao inserir matricula na arvore";
			return false;
		}

		return true;
	}

	auto DataManager::getEnrollment(int32_t studentId, int32_t subjectId) const
		-> std::optional<project_model::BTreeLeafValue>
	{
		return btree_.search(enrollmentKey(studentId, subjectId));
	}

	auto DataManager::updateGrade(int32_t studentId, int32_t subjectId, float grade) -> bool {
		using namespace project_model;

		auto key = enrollmentKey(studentId, subjectId);
		auto opt = btree_.search(key);
		if (!opt) {
			lastError_ = "Matricula nao encontrada";
			return false;
		}

		// Remove old entry and re-insert with updated grade
		auto val = *opt;
		val.grade = grade;
		btree_.erase(key);
		btree_.insert(key, val);
		return true;
	}

	auto DataManager::unenroll(int32_t studentId, int32_t subjectId) -> bool {
		auto key = enrollmentKey(studentId, subjectId);
		if (!btree_.erase(key)) {
			lastError_ = "Matricula nao encontrada";
			return false;
		}
		return true;
	}

	auto DataManager::getEnrollmentsByStudent(int32_t studentId) const
		-> std::vector<project_model::BTreeLeafValue>
	{
		auto prefix = enrollmentsByStudentPrefix(studentId);
		auto results = btree_.searchRange(prefix);
		std::vector<project_model::BTreeLeafValue> out;
		out.reserve(results.size());
		for (auto& [_, val] : results)
			out.push_back(val);
		return out;
	}

	auto DataManager::getEnrollmentsBySubject(int32_t subjectId) const
		-> std::vector<project_model::BTreeLeafValue>
	{
		auto prefix = std::string("ENR:SUB:") + padId(subjectId) + ":";
		// We need to scan all enrollments and filter by subject
		// Since our key is ENR:STU:XXXX:SUB:YYYY, scanning by SUB prefix isn't
		// efficient with this key scheme. Let's do a full range scan and filter.
		auto results = btree_.searchRange("ENR:");
		std::vector<project_model::BTreeLeafValue> out;
		out.reserve(results.size());
		for (auto& [_, val] : results) {
			if (val.subjectId == subjectId)
				out.push_back(val);
		}
		return out;
	}

	// ========================================
	// Info methods
	// ========================================
	auto DataManager::getNextStudentId() const -> int32_t {
		return static_cast<int32_t>(indexCtrl_.nextId("STU:"));
	}

	auto DataManager::getNextTeacherId() const -> int32_t {
		return static_cast<int32_t>(indexCtrl_.nextId("TCH:"));
	}

	auto DataManager::getNextSubjectId() const -> int32_t {
		return static_cast<int32_t>(indexCtrl_.nextId("SUB:"));
	}

	auto DataManager::getActiveCount(char type) const -> int32_t {
		int ci = chunkIndexForType(type);
		if (ci < 0) return 0;
		return static_cast<int32_t>(countActiveInChunk(ci));
	}

} // namespace project_controller

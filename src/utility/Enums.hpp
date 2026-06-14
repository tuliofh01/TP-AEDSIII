#pragma once

#include <cstdint>

namespace project_utility {

	// Record lifecycle: 'A' = active, '*' = soft-deleted.
	// Deletion flips the status byte; the record is never physically removed.
	// This keeps chunk-level record indices stable over time.
	enum class RecStatus : char {
		Ativo = 'A',
		Deletado = '*'
	};
	using enum RecStatus;

	// Entity type byte embedded in every record header.
	// Used by DataManager::chunkIndexForType() to map a type char to a chunk slot.
	// 'I' is reserved for the B+ Tree data (not a user-facing entity).
	enum class RecType : char {
		Student = 'S',
		Teacher = 'T',
		Subject = 'B',
		Tree = 'I'
	};
	using enum RecType;

	// Canonical CRUD + enrollment operations (currently unused in C++,
	// kept as reference for potential audit-logging or operation routing).
	enum class CrudOp : uint8_t {
		Create = 0,
		Read = 1,
		Update = 2,
		Delete = 3,
		List = 4,
		Search = 5,
		Enroll = 6,
		Unenroll = 7
	};

	// View identifiers matching router.lua navigation states.
	// Kept in C++ for potential future use in Lua push guards or permission checks.
	enum class ViewId : uint8_t {
		Menu = 0,
		StudentCreate = 1,
		StudentList = 2,
		StudentDetail = 3,
		StudentSearch = 4,
		TeacherCreate = 5,
		TeacherList = 6,
		TeacherDetail = 7,
		SubjectCreate = 8,
		SubjectList = 9,
		SubjectDetail = 10,
		EnrollmentList = 11,
		EnrollmentByStudent = 12,
		EnrollmentBySubject = 13
	};
	using enum ViewId;

	// The 4-color minimal theme used by common.lua for ImGui styling.
	enum class ThemeColor : uint8_t {
		Black = 0,
		White = 1,
		Red = 2,
		Green = 3
	};
	using enum ThemeColor;

} // namespace project_utility

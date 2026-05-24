#pragma once

#include <cstdint>

namespace project_utility {

	enum class RecStatus : char {
		Ativo = 'A',
		Deletado = '*'
	};
	using enum RecStatus;

	enum class RecType : char {
		Student = 'S',
		Teacher = 'T',
		Subject = 'B',
		Tree = 'I'
	};
	using enum RecType;

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

	enum class ThemeColor : uint8_t {
		Black = 0,
		White = 1,
		Red = 2,
		Green = 3
	};
	using enum ThemeColor;

} // namespace project_utility

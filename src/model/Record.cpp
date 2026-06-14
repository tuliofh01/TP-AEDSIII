// Record.cpp — thin serialization wrappers.
// FileHeader and ChunkInfo have their own toBytes/fromBytes because they are
// not wrapped by model classes (unlike StudentRecord/TeacherRecord/etc.).
// The actual byte-copying happens in the generic templates: serializeRecord<T>()
// and deserializeRecord<T>() from Record.hpp — single memcpy each.
//
// All other packed records (StudentRecord, TeacherRecord, SubjectRecord,
// BTreeLeafValue) are serialized directly via those templates in DataManager.cpp.

#include "Record.hpp"
#include <cstring>

namespace project_model {

std::vector<std::byte> FileHeader::toBytes() const {
	return serializeRecord(*this);
}

FileHeader FileHeader::fromBytes(const std::vector<std::byte>& data) {
	return deserializeRecord<FileHeader>(data);
}

std::vector<std::byte> ChunkInfo::toBytes() const {
	return serializeRecord(*this);
}

ChunkInfo ChunkInfo::fromBytes(const std::vector<std::byte>& data) {
	return deserializeRecord<ChunkInfo>(data);
}

} // namespace project_model

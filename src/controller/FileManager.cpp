#include "FileManager.hpp"

namespace project_controller {

	FileManager::FileManager(size_t recordSize)
		: recordSize_(recordSize) {}

	auto FileManager::initialize(const std::string& path) -> bool {
		filePath_ = path;
		fileStream_.open(filePath_, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
		if (!fileStream_.is_open()) {
			fileStream_.open(filePath_, std::ios::binary | std::ios::out | std::ios::trunc);
			if (!fileStream_.is_open()) return false;
		}
		updateCount();
		return true;
	}

	auto FileManager::isOpen() const -> bool {
		return fileStream_.is_open();
	}

	auto FileManager::calcOffset(size_t index) const -> size_t {
		return index * recordSize_;
	}

	auto FileManager::readAt(std::vector<std::byte>& buf, size_t off) const -> bool {
		if (buf.size() < recordSize_) buf.resize(recordSize_);
		fileStream_.seekg(static_cast<std::streamoff>(off), std::ios::beg);
		if (!fileStream_.good()) return false;
		fileStream_.read(reinterpret_cast<char*>(buf.data()), recordSize_);
		return fileStream_.good() || fileStream_.eof();
	}

	auto FileManager::writeAt(const std::vector<std::byte>& data, size_t off) const -> bool {
		if (data.size() < recordSize_) return false;
		fileStream_.seekp(static_cast<std::streamoff>(off), std::ios::beg);
		if (!fileStream_.good()) return false;
		fileStream_.write(reinterpret_cast<const char*>(data.data()), recordSize_);
		return fileStream_.good();
	}

	void FileManager::updateCount() {
		fileStream_.seekg(0, std::ios::end);
		if (fileStream_.fail()) { recordCount_ = 0; return; }
		auto fileSize = fileStream_.tellg();
		recordCount_ = (fileSize > 0) ? static_cast<size_t>(fileSize) / recordSize_ : 0;
		fileStream_.seekg(0, std::ios::beg);
	}

	auto FileManager::readRecord(size_t index) const -> std::optional<std::vector<std::byte>> {
		std::vector<std::byte> buf(recordSize_);
		if (!readAt(buf, calcOffset(index))) return std::nullopt;
		return buf;
	}

	auto FileManager::writeRecord(const std::vector<std::byte>& data, size_t index) -> bool {
		return writeAt(data, calcOffset(index));
	}

	auto FileManager::appendRecord(const std::vector<std::byte>& data) -> bool {
		if (data.size() < recordSize_) return false;
		fileStream_.seekp(0, std::ios::end);
		if (!fileStream_.good()) return false;
		fileStream_.write(reinterpret_cast<const char*>(data.data()), recordSize_);
		if (!fileStream_.good()) return false;
		++recordCount_;
		return true;
	}

	auto FileManager::countActive() const -> uint32_t {
		using namespace project_utility;
		uint32_t count = 0;
		std::vector<std::byte> buf(recordSize_);
		for (size_t i = 0; i < recordCount_; ++i) {
			if (readAt(buf, calcOffset(i))) {
				auto s = std::to_integer<char>(buf[0]);
				if (s == static_cast<char>(RecStatus::Ativo)) ++count;
			}
		}
		return count;
	}

	auto FileManager::countTotal() const -> uint32_t {
		return static_cast<uint32_t>(recordCount_);
	}

	auto FileManager::markDeleted(size_t index) const -> bool {
		if (index >= recordCount_) return false;
		std::vector<std::byte> buf(recordSize_);
		if (!readAt(buf, calcOffset(index))) return false;
		buf[0] = static_cast<std::byte>(RecStatus::Deletado);
		return writeAt(buf, calcOffset(index));
	}

	auto FileManager::isDeleted(size_t index) const -> bool {
		if (index >= recordCount_) return true;
		std::vector<std::byte> buf(recordSize_);
		if (!readAt(buf, calcOffset(index))) return true;
		return std::to_integer<char>(buf[0]) == static_cast<char>(RecStatus::Deletado);
	}

	void FileManager::flush() const {
		if (fileStream_.is_open()) fileStream_.flush();
	}

	void FileManager::close() {
		if (fileStream_.is_open()) fileStream_.close();
		recordCount_ = 0;
	}

} // namespace project_controller

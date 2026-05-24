#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <fstream>

#include "../model/Record.hpp"

namespace project_controller {

	class FileManager {
	public:
		FileManager() = default;
		explicit FileManager(size_t recordSize);

		~FileManager() = default;
		FileManager(const FileManager&) = delete;
		FileManager& operator=(const FileManager&) = delete;
		FileManager(FileManager&&) noexcept = default;
		FileManager& operator=(FileManager&&) noexcept = default;

		[[nodiscard]] bool initialize(const std::string& path);
		[[nodiscard]] bool isOpen() const;

		void setRecordSize(size_t recordSize) { recordSize_ = recordSize; }
		[[nodiscard]] size_t getRecordSize() const { return recordSize_; }

		[[nodiscard]] std::optional<std::vector<std::byte>> readRecord(size_t index) const;
		[[nodiscard]] bool writeRecord(const std::vector<std::byte>& data, size_t index);
		[[nodiscard]] bool appendRecord(const std::vector<std::byte>& data);

		[[nodiscard]] uint32_t countActive() const;
		[[nodiscard]] uint32_t countTotal() const;

		[[nodiscard]] bool markDeleted(size_t index) const;
		[[nodiscard]] bool isDeleted(size_t index) const;

		void flush() const;
		void close();

		[[nodiscard]] bool readAt(std::vector<std::byte>& buf, size_t off) const;
		[[nodiscard]] bool writeAt(const std::vector<std::byte>& data, size_t off) const;

	private:
		mutable std::fstream fileStream_;
		std::string filePath_;
		size_t recordCount_ = 0;
		size_t recordSize_ = project_utility::RECORD_TOTAL_SIZE;

		[[nodiscard]] size_t calcOffset(size_t index) const;
		void updateCount();
	};

} // namespace project_controller

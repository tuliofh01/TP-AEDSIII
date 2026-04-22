/**
 * @file Enums.hpp
 * @brief Enum definitions for system status, operations, and views.
 * @desc Defines all enumerations used throughout the system. Each enum
 *       includes 'using enum' declaration for direct access to values
 *       without namespace prefix (C++20 feature). This simplifies code
 *       and makes enum values feel like they belong to the namespace.
 * @author TP-AEDSIII Team
 * @date 2026-04-22 (backdated)
 * @namespace project_utility - Flat namespace for enums.
 */
#pragma once

#include <cstdint>

/**
 * @namespace project_utility
 * @brief Flat namespace for utility types including enums.
 * @details All enums use 'using enum' to allow direct access to values.
 *          Example: instead of RecStatus::Ativo, just use Ativo.
 */
namespace project_utility {

	// ========================================
	// Record Status Enum (binary file)
	// ========================================
	/**
	 * @enum RecStatus
	 * @brief Status byte values for binary records.
	 * @desc Indicates whether a record is active or logically deleted.
	 *       Stored as single byte (char) in first position of each record.
	 */
	enum class RecStatus : char {
		/**
		 * @brief Active/valid record status.
		 * @details Record is valid and should be included in listings.
		 *          Stored as ASCII 'A' character.
		 */
		Ativo = 'A',

		/**
		 * @brief Deleted record status (soft delete).
		 * @details Record was logically deleted but data still exists on disk.
		 *          Stored as ASCII '*' character. This allows recovery and
		 *          avoids rewriting entire file.
		 * @see FileManager::markDeleted()
		 */
		Deletado = '*'
	};

	/**
	 * @brief Import all RecStatus values into namespace scope.
	 * @desc C++20 feature that allows using Ativo and Deletado directly
	 *       without RecStatus:: prefix. Makes code cleaner and shorter.
	 */
	using enum RecStatus;

	// ========================================
	// CRUD Operations Enum
	// ========================================
	/**
	 * @enum CrudOp
	 * @brief Types of CRUD operations supported by DataManager.
	 * @desc Used for logging, debugging, and operation tracking.
	 */
	enum class CrudOp : uint8_t {
		/** @brief Create new record operation. */
		Create = 0,

		/** @brief Read/existing record operation. */
		Read = 1,

		/** @brief Update existing record operation. */
		Update = 2,

		/** @brief Delete record (soft delete) operation. */
		Delete = 3,

		/** @brief List all active records operation. */
		List = 4,

		/** @brief Search records by criteria operation. */
		Search = 5
	};

	// ========================================
	// GUI View IDs Enum
	// ========================================
	/**
	 * @enum ViewId
	 * @brief Unique identifiers for each GUI screen/view.
	 * @desc Used by router.lua to determine which view to render.
	 *       Each value corresponds to a Lua view file.
	 */
	enum class ViewId : uint8_t {
		/** @brief Main menu screen. */
		Menu = 0,

		/** @brief Student creation form screen. */
		StudentCreate = 1,

		/** @brief Student list/table screen. */
		StudentList = 2,

		/** @brief Student detail/edit screen. */
		StudentDetail = 3,

		/** @brief Student search screen. */
		StudentSearch = 4
	};

	/**
	 * @brief Import all ViewId values into namespace scope.
	 * @desc Allows direct access like Menu, StudentCreate without ViewId:: prefix.
	 */
	using enum ViewId;

	// ========================================
	// Theme Colors Enum (4-color minimalist)
	// ========================================
	/**
	 * @enum ThemeColor
	 * @brief Predefined color palette for 4-color theme.
	 * @desc Minimalist theme uses exactly 4 colors: Black, White, Red, Green.
	 *       No other colors should be used in UI to maintain consistency.
	 * @see docs/ux/specs/interface_spec.md
	 */
	enum class ThemeColor : uint8_t {
		/** @brief Primary text and border color. */
		Black = 0,

		/** @brief Background color. */
		White = 1,

		/** @brief Error and delete button color. */
		Red = 2,

		/** @brief Success and primary action color. */
		Green = 3
	};

	/**
	 * @brief Import all ThemeColor values into namespace scope.
	 */
	using enum ThemeColor;

} // namespace project_utility

/* ========================================================================
 * END OF FILE - Enums.hpp
 * Purpose: Define all enum types for system state and operations.
 * Dependencies: <cstdint> for fixed-width integer types.
 * Used By: Record.hpp, DataManager.hpp, FileManager.cpp, router.lua.
 * ======================================================================== */
/**
 * @file ErrorCodes.h
 * @brief Centralized error code definitions for API responses
 * 
 * This file provides a single source of truth for all error codes used
 * in API responses. Using constants instead of magic strings provides:
 * - Compile-time checking (typos become compile errors)
 * - IDE autocomplete support
 * - Easy refactoring (change format in one place)
 * - Clear documentation of all possible errors
 * 
 * Error codes follow the pattern: "category/specific_error"
 * 
 * Usage example:
 * @code
 * #include "system/ErrorCodes.h"
 * 
 * root["error"] = ErrorCodes::Config::NOT_CONFIGURED;
 * root["error"] = ErrorCodes::Input::EMPTY_TEXT;
 * @endcode
 */

#pragma once

namespace ErrorCodes {

/**
 * Configuration-related errors
 * Used when service or feature is not properly configured
 */
namespace Config {
    constexpr const char* NOT_CONFIGURED = "config/not_configured";
    constexpr const char* INVALID_VALUE = "config/invalid_value";
}

/**
 * Input validation errors
 * Used when user-provided input fails validation
 */
namespace Input {
    constexpr const char* JSON_PARSE_ERROR = "input/json_parse_error";
    constexpr const char* EMPTY_TEXT = "input/empty_text";
    constexpr const char* TEXT_TOO_LONG = "input/text_too_long";
    constexpr const char* INVALID_RANGE = "input/invalid_range";
    constexpr const char* INVALID_FORMAT = "input/invalid_format";
}

/**
 * Service availability errors
 * Used when a required service is unavailable or not initialized
 */
namespace Service {
    constexpr const char* UNAVAILABLE = "service/unavailable";
    constexpr const char* TELEGRAM_SETTINGS_UNAVAILABLE = 
        "service/telegram_settings_unavailable";
}

/**
 * Busy/concurrency errors
 * Used when resource is temporarily unavailable due to concurrent access
 */
namespace Busy {
    constexpr const char* TELEGRAM_TEST_IN_PROGRESS = 
        "busy/telegram_test_in_progress";
    constexpr const char* FILESYSTEM_BUSY = "busy/filesystem";
    constexpr const char* RESOURCE_LOCKED = "busy/resource_locked";
}

/**
 * Internal/system errors
 * Used for unexpected internal failures (allocation, task creation, etc.)
 */
namespace Internal {
    constexpr const char* ALLOC_FAILED = "internal/alloc_failed";
    constexpr const char* TASK_CREATE_FAILED = "internal/task_create_failed";
    constexpr const char* TASK_TIMEOUT = "internal/task_timeout";
    constexpr const char* UNEXPECTED_ERROR = "internal/unexpected_error";
}

}  // namespace ErrorCodes

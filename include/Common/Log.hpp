#pragma once
#include <fmt/format.h>
#include <fmt/color.h>
#include <filesystem>

#define DEBUG_LOGS
#ifdef DEBUG_LOGS
#define LOG_TRACE(format_value, ...) fmt::println("{}", fmt::format(fmt::fg(fmt::color::white), "[{} - LN {} ({})] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, TARGET_LOG_NAME, fmt::format(format_value, __VA_ARGS__)))
#define LOG_WARN(format_value, ...) fmt::println("{}", fmt::format(fmt::fg(fmt::color::yellow), "[{} - LN {} ({})] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, TARGET_LOG_NAME, fmt::format(format_value, __VA_ARGS__)))
#define LOG_ERROR(format_value, ...) fmt::println("{}", fmt::format(fmt::fg(fmt::color::red), "[{} - LN {} ({})] {}", std::filesystem::path(__FILE__).filename().string(), __LINE__, TARGET_LOG_NAME, fmt::format(format_value, __VA_ARGS__)))
#else
#define LOG_TRACE(format_value, ...) fmt::println("{}", fmt::format(fmt::fg(fmt::color::white), "[{}] {}", TARGET_LOG_NAME, fmt::format(format_value, __VA_ARGS__)))
#define LOG_WARN(format_value, ...) fmt::println("{}", fmt::format(fmt::fg(fmt::color::yellow), "[{}] {}", TARGET_LOG_NAME, fmt::format(format_value, __VA_ARGS__)))
#define LOG_ERROR(format_value, ...) fmt::println("{}", fmt::format(fmt::fg(fmt::color::red), "[{}] {}", TARGET_LOG_NAME, fmt::format(format_value, __VA_ARGS__)))
#endif
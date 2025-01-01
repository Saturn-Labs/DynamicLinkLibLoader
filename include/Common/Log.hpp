#pragma once
#include <fmt/format.h>
#include <fmt/color.h>

#define LOG_TRACE(format_value, ...) fmt::println("{}", fmt::format(fmt::fg(fmt::color::white), "[{}] {}", TARGET_LOG_NAME, fmt::format(format_value, __VA_ARGS__)))
#define LOG_WARN(format_value, ...) fmt::println("{}", fmt::format(fmt::fg(fmt::color::yellow), "[{}] {}", TARGET_LOG_NAME, fmt::format(format_value, __VA_ARGS__)))
#define LOG_ERROR(format_value, ...) fmt::println("{}", fmt::format(fmt::fg(fmt::color::red), "[{}] {}", TARGET_LOG_NAME, fmt::format(format_value, __VA_ARGS__)))
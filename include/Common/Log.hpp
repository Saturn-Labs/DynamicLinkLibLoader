#pragma once
#include <fmt/format.h>
#include <fmt/color.h>

#define LOG_TRACE(format_value, ...) fmt::println("{}", fmt::format(fmt::fg(fmt::color::white), format_value, __VA_ARGS__))
#define LOG_WARN(format_value, ...) fmt::println("{}", fmt::format(fmt::fg(fmt::color::yellow), format_value, __VA_ARGS__))
#define LOG_ERROR(format_value, ...) fmt::println("{}", fmt::format(fmt::fg(fmt::color::red), format_value, __VA_ARGS__))
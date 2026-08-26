#ifndef _CLAMSHELL_LOG_HH
#define _CLAMSHELL_LOG_HH

#include <print>
#include <format>
#include <string_view>
#include <source_location>

namespace detail {
  template <typename ...Args>
  void log(
    std::FILE* stream,
    std::string_view esc,
    std::string_view category,
    std::source_location location,
    std::format_string<Args...> fmt,
    const Args&... args
  ) noexcept {
    std::println(
      stream,
      "{}[clamshell::{} {}:{}]\033[0m {}",
      esc,
      category,
      location.line(),
      location.column(),
      std::vformat(fmt.get(), std::make_format_args(args...))
    );
    std::fflush(stream);
  }
}


#define CLAMSHELL_INFO(fmt, ...) detail::log(stdout, "\x1b[32m", "info", std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define CLAMSHELL_TRACE(fmt, ...) detail::log(stdout, "\x1b[34m", "trace", std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define CLAMSHELL_WARN(fmt, ...) detail::log(stdout, "\x1b[33m", "warn", std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define CLAMSHELL_ERROR(fmt, ...) detail::log(stderr, "\x1b[31m", "error", std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)
#define CLAMSHELL_FATAL(fmt, ...) detail::log(stderr, "\x1b[31m", "fatal", std::source_location::current(), fmt __VA_OPT__(, ) __VA_ARGS__)

#endif

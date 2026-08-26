#ifndef _CLAMSHELL_UTILS_HH
#define _CLAMSHELL_UTILS_HH

#include <unistd.h>
#include <fcntl.h>

#include <string_view>
#include <cerrno>
#include <cstring>

#include "log.hh"

namespace utils {
  constexpr int invalid_fd = -1;

  struct unique_fd {
    int fd = invalid_fd;

    unique_fd(int fd) noexcept;
    ~unique_fd() noexcept;

    unique_fd(const unique_fd&) = delete;
    unique_fd& operator=(const unique_fd&) = delete;

    unique_fd(unique_fd&& other) noexcept;
    unique_fd& operator=(unique_fd&& other) noexcept;

    operator int() const noexcept;
    operator bool() const noexcept;

    auto operator<=>(int rhs) const noexcept;
    bool operator==(int rhs) const noexcept;
  };

  inline bool write_file(const char* path, std::string_view value) noexcept {
    unique_fd fd(open(path, O_WRONLY | O_CLOEXEC));
    if (!fd) {
      CLAMSHELL_ERROR("failed to open \"{}\": {}", path, std::strerror(errno));
      return false;
    }
    if (::write(fd, value.data(), value.size()) != static_cast<ssize_t>(value.size())) {
      CLAMSHELL_ERROR(
        "failed to write \"{}\" to \"{}\": {}",
        value,
        path,
        std::strerror(errno)
      );
      return false;
    }
    return true;
  }
}

#endif

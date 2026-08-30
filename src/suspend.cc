#include <array>
#include <fstream>
#include <sstream>
#include <string>

#include <unistd.h>

#include "utils.hh"
#include "suspend.hh"
#include "config.hh"
#include "nvidia.hh"
#include "log.hh"
#include "vt.hh"

namespace {
  constexpr const char* power_state_path = "/sys/power/state";
  constexpr const char* mem_power_state_path = "/sys/power/mem_sleep";

  config::suspend_mode use_suspend_mode;

  struct sleep_caps {
    bool freeze : 1;
    bool standby : 1;
    bool mem: 1;
    bool disk: 1;
    bool _ : 4;
  };

  struct mem_sleep_caps {
    bool s2idle : 1;
    bool shallow : 1;
    bool deep: 1;
    bool _ : 5;
  };

  void get_sleep_cap(sleep_caps& caps) noexcept {
    std::ifstream file(power_state_path);

    if (file.is_open()) {
      std::string state;
      std::string part;
      std::getline(file, state);
      std::istringstream iss(state);

      while (iss >> part) {
        if (part == "freeze") {
          caps.freeze = 1;
        } else if (part == "standby") {
          caps.standby = 1;
        } else if (part == "mem") {
          caps.mem = 1;
        } else if (part == "disk") {
          caps.disk = 1;
        }
      }
    }
  }

  void get_mem_sleep_cap(mem_sleep_caps& caps) noexcept {
    std::ifstream file(mem_power_state_path);

    if (file.is_open()) {
      std::string state;
      std::string part;
      std::getline(file, state);
      std::istringstream iss(state);

      while (iss >> part) {
        if (!part.empty() && part.front() == '[') {
          part = part.substr(1, part.size() - 2);
        }

        if (part == "s2idle") {
          caps.s2idle = 1;
        } else if (part == "shallow") {
          caps.shallow = 1;
        } else if (part == "deep") {
          caps.deep = 1;
        }
      }
    }
  }

  bool is_mode_available(
    config::suspend_mode mode,
    const sleep_caps& scaps,
    const mem_sleep_caps& mcaps
  ) noexcept {
    switch (mode) {
      using enum config::suspend_mode;

      case freeze: return scaps.freeze;
      case suspend_to_ram: return scaps.mem && mcaps.deep;
      case suspend_to_disk: return scaps.disk;
    }
    return false;
  }

  void exec_freeze() noexcept {
    CLAMSHELL_TRACE("execute suspend with \033[1mfreeze\033[22m");
    utils::write_file(power_state_path, "freeze");
  }

  void exec_suspend_to_ram() noexcept {
    CLAMSHELL_TRACE("execute suspend with \033[1msuspend to ram\033[22m");

    if (!utils::write_file(mem_power_state_path, "deep")) {
      return;
    }

    utils::write_file(power_state_path, "mem");
  }

  void exec_suspend_to_disk() noexcept {
    CLAMSHELL_TRACE("execute suspend with \033[1msuspend to disk\033[22m");
    utils::write_file(power_state_path, "disk");
  }
}

bool check_suspend_caps() noexcept {
  sleep_caps sleep_caps{};
  mem_sleep_caps mem_caps{};

  get_sleep_cap(sleep_caps);
  CLAMSHELL_INFO(
    "sleep caps {{\n  freeze = \033[1m{}\033[22m\n  standby = \033[1m{}\033[22m\n  mem = \033[1m{}\033[22m\n  disk = \033[1m{}\033[22m\n}}",
    sleep_caps.freeze,
    sleep_caps.standby,
    sleep_caps.mem,
    sleep_caps.disk
  );

  get_mem_sleep_cap(mem_caps);
  CLAMSHELL_INFO(
    "mem_sleep_caps {{\n  s2idle = \033[1m{}\033[22m\n  shallow = \033[1m{}\033[22m\n  deep = \033[1m{}\033[22m\n}}",
    mem_caps.s2idle,
    mem_caps.shallow,
    mem_caps.deep
  );

  if (!config::fallback) {
    use_suspend_mode = config::suspend_mode_type;
    return is_mode_available(config::suspend_mode_type, sleep_caps, mem_caps);
  }

  using mode = config::suspend_mode;

  static constexpr std::array<std::array<mode, 3>, 3> fallbacks {{
    { mode::freeze, mode::suspend_to_disk, mode::suspend_to_ram },
    { mode::suspend_to_ram, mode::suspend_to_disk, mode::freeze },
    { mode::suspend_to_disk, mode::suspend_to_ram, mode::freeze },
  }};

  const auto& order = fallbacks[
    static_cast<std::size_t>(config::suspend_mode_type)
  ];

  const auto it = std::ranges::find_if(
    order,
    [&sleep_caps, &mem_caps](config::suspend_mode mode) {
      return is_mode_available(mode, sleep_caps, mem_caps);
    }
  );

  if (it != order.end()) {
    use_suspend_mode = *it;

    if (*it != config::suspend_mode_type) {
      CLAMSHELL_WARN(
        "suspend mode \"{}\" is not available, falling back to \"{}\"",
        config::to_string(config::suspend_mode_type),
        config::to_string(*it)
      );
    }

    return true;
  }

  return false;
}

void suspend() noexcept {
  sync();

  int prev = vt::current();
  if (prev == utils::invalid_fd) {
    return;
  }

  if (vt::change(vt::parking) == utils::invalid_fd) {
    vt::change(prev);
    return;
  }

  nvidia::suspend(use_suspend_mode);

  switch (use_suspend_mode) {
    using enum config::suspend_mode;

    case freeze: exec_freeze(); break;
    case suspend_to_ram: exec_suspend_to_ram(); break;
    case suspend_to_disk: exec_suspend_to_disk(); break;
  }

  nvidia::resume(use_suspend_mode);
  vt::change(prev);
}

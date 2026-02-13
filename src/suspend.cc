#include <fstream>
#include <sstream>
#include <string>

#include <unistd.h>

#include "suspend.hh"

namespace cramshell {
  suspend_caps get_suspend_caps() noexcept {
    std::ifstream file("/sys/power/state");

    suspend_caps caps {
      .freeze = 0,
      .standby = 0,
      .mem = 0,
      .disk = 0,
    };

    if (!file.is_open()) {
      return caps;
    }

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
    file.close();

    return caps;
  }

  void suspend(suspend_type type) noexcept {
    sync();

    std::ofstream nvidia_state("/proc/driver/nvidia/suspend");
    if (nvidia_state.is_open()) {
      nvidia_state << "suspend";
      nvidia_state.close();
    }

    std::ofstream state("/sys/power/state");
    if (state.is_open()) {
      switch (type) {
        case suspend_type::freeze:
          state << "freeze";
          break;
        case suspend_type::standby:
          state << "standby";
          break;
        case suspend_type::mem:
          state << "mem";
          break;
        case suspend_type::disk:
          state << "disk";
          break;
      }
      state.close();
    }
  }

  void resume() noexcept {
    std::ofstream nvidia_state("/proc/driver/nvidia/suspend");
    if (nvidia_state.is_open()) {
      nvidia_state << "resume";
      nvidia_state.close();
    }
  }
}

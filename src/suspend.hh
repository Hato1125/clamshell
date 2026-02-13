#ifndef _CRAMSHELL_SUSPEND_HH
#define _CRAMSHELL_SUSPEND_HH

namespace cramshell {
  bool check_suspend_caps() noexcept;
  void suspend() noexcept;
  void resume() noexcept;
}

#endif

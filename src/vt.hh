#ifndef _CLAMSHELL_VT_HH
#define _CLAMSHELL_VT_HH

namespace vt {
  int current() noexcept;
  int available() noexcept;
  int change(int vt) noexcept;
}

#endif

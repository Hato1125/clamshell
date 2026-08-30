#ifndef _CLAMSHELL_VT_HH
#define _CLAMSHELL_VT_HH

namespace vt {
  // VT used to park the compositor while the GPU is suspended.
  //
  // A fixed number is used instead of VT_OPENQRY: VT_OPENQRY returns the
  // first unopened VT (usually tty2), but logind spawns autovt@ttyN (a
  // login prompt) whenever tty2-tty6 becomes active, so every suspend
  // would leak one getty and shift the next query to the next VT.
  //
  // 63 is MAX_NR_CONSOLES: outside the default autovt range, normally not
  // assigned a static getty, and allocated on demand by VT_ACTIVATE. It is
  // also the VT nvidia-sleep.sh parks on, so in official_script mode both
  // switches land on the same VT and the second one becomes a no-op.
  constexpr int parking = 63;

  int current() noexcept;
  int change(int vt) noexcept;
}

#endif

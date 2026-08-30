#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/vt.h>

#include "utils.hh"
#include "vt.hh"

namespace vt {
  int current() noexcept {
    if (auto fd = utils::unique_fd(open("/dev/tty0", O_RDWR | O_CLOEXEC)); fd) {
      struct vt_stat st;
      int ret = ioctl(fd, VT_GETSTATE, &st);
      return ret < 0 ? -1 : st.v_active;
    }
    return utils::invalid_fd;
  }

  int available() noexcept {
    if (auto fd = utils::unique_fd(open("/dev/tty0", O_RDWR | O_CLOEXEC)); fd) {
      int target = utils::invalid_fd;

      if (ioctl(fd, VT_OPENQRY, &target) == 0) {
        return target;
      }
    }

    return utils::invalid_fd;
  }

  int change(int vt) noexcept {
    if (auto fd = utils::unique_fd(open("/dev/tty0", O_RDWR | O_CLOEXEC)); fd) {
      if (ioctl(fd, VT_ACTIVATE, vt) == 0
        && ioctl(fd, VT_WAITACTIVE, vt) == 0) {
        return 0;
      }
    }
    return -1;
  }
}

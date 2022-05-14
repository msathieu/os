#include <__/syscall.h>
#include <capability.h>
#include <ipccalls.h>
#include <memory.h>
#include <string.h>
#include <syslog.h>

static size_t width;
static size_t height;
static size_t pitch;
static size_t bits_per_pixel;
static int red_index;
static int green_index;
static int blue_index;
static char* framebuffer = (char*) PHYSICAL_MAPPINGS_START;

static int64_t info_handler(uint64_t info, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, __attribute__((unused)) uint64_t arg3, __attribute__((unused)) uint64_t arg4) {
  if (!has_ipc_caller_capability(CAP_NAMESPACE_DRIVERS, CAP_FBD_DRAW)) {
    syslog(LOG_DEBUG, "No permission to access framebuffer data");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  switch (info) {
  case 0:
    return width;
    break;
  case 1:
    return height;
    break;
  case 2:
    return pitch;
    break;
  case 3:
    return bits_per_pixel;
    break;
  case 4:
    return red_index;
    break;
  case 5:
    return green_index;
    break;
  case 6:
    return blue_index;
    break;
  default:
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
}
static int64_t copy_handler(uint64_t offset, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, uint64_t address, uint64_t size) {
  if (!has_ipc_caller_capability(CAP_NAMESPACE_DRIVERS, CAP_FBD_DRAW)) {
    syslog(LOG_DEBUG, "No permission to write to framebuffer");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  size_t total_size;
  if (__builtin_uaddl_overflow(offset, size, &total_size)) {
    syslog(LOG_DEBUG, "Can't write this much data to framebuffer");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (total_size > height * pitch) {
    syslog(LOG_DEBUG, "Can't write this much data to framebuffer");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  memcpy(framebuffer + offset, (void*) address, size);
  return 0;
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc();
  width = _syscall(_SYSCALL_GET_FB_INFO, 1, 0, 0, 0, 0);
  height = _syscall(_SYSCALL_GET_FB_INFO, 2, 0, 0, 0, 0);
  pitch = _syscall(_SYSCALL_GET_FB_INFO, 3, 0, 0, 0, 0);
  bits_per_pixel = _syscall(_SYSCALL_GET_FB_INFO, 4, 0, 0, 0, 0);
  red_index = _syscall(_SYSCALL_GET_FB_INFO, 5, 0, 0, 0, 0);
  green_index = _syscall(_SYSCALL_GET_FB_INFO, 6, 0, 0, 0, 0);
  blue_index = _syscall(_SYSCALL_GET_FB_INFO, 7, 0, 0, 0, 0);
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_GET_FB_INFO);
  memset(framebuffer, 0, height * pitch);
  register_ipc_call(IPC_FBD_INFO, info_handler, 1);
  register_ipc_call(IPC_FBD_COPY, copy_handler, 1);
  while (true) {
    handle_ipc();
  }
}

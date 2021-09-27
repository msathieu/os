#include <capability.h>
#include <ipccalls.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <tty/fb.h>
#include <tty/kbd.h>

static int64_t print_handler(__attribute__((unused)) uint64_t offset, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  if (arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  size_t fb = 0;
  if (has_ipc_caller_capability(CAP_NAMESPACE_SERVERS, CAP_LOGD)) {
    fb = 11;
  } else if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_DEVD)) {
    syslog(LOG_DEBUG, "Not allowed to access tty");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  for (size_t i = 0; i < size; i++) {
    if (((char*) address)[i]) {
      put_character(((char*) address)[i], fb);
    }
  }
  update_fb();
  return size;
}
static int64_t kbd_input_handler(__attribute__((unused)) uint64_t offset, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  if (arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_DEVD)) {
    syslog(LOG_DEBUG, "Not allowed to access tty");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  bool block = 1;
  for (size_t i = 0; i < size; i++) {
    char c = get_character();
    if (c) {
      block = 0;
      ((char*) address)[i] = c;
    } else {
      if (block) {
        call_blocked = 1;
        return -IPC_ERR_BLOCK;
      } else {
        return i;
      }
    }
  }
  return size;
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(1);
  ipc_set_started();
  setup_fb();
  setup_kbd();
  send_ipc_call("logd", IPC_LOGD_REGISTER, 0, 0, 0, 0, 0);
  drop_capability(CAP_NAMESPACE_SERVERS, CAP_LOGD_TTY);
  setenv("PATH", "/bin", 0);
  fopen("/dev/tty", "r");
  fopen("/dev/tty", "w");
  fopen("/dev/tty", "w");
  spawn_process("/bin/sh");
  start_process();
  ipc_handlers[IPC_VFSD_FS_WRITE] = print_handler;
  ipc_handlers[IPC_VFSD_FS_READ] = kbd_input_handler;
  while (1) {
    handle_ipc();
  }
}

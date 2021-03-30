#include <capability.h>
#include <ipccalls.h>
#include <spawn.h>
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
    // TODO: Remove CAP_VFSD
  } else if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD) && !has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_DEVD)) {
    syslog(LOG_DEBUG, "Not allowed to access tty");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  for (size_t i = 0; i < size && ((char*) address)[i]; i++) {
    put_character(((char*) address)[i], fb);
  }
  update_fb();
  return 0;
}
static int64_t kbd_input_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg0 || arg1 || arg2 || arg3 || arg4) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (!is_caller_child()) {
    syslog(LOG_DEBUG, "No permission to get keyboard input");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  if (call_blocked) {
    syslog(LOG_DEBUG, "Only one program can request keyboard input simultaneously");
    return -IPC_ERR_PROGRAM_DEFINED;
  }
  char c = get_character();
  if (!c) {
    call_blocked = 1;
    return -IPC_ERR_BLOCK;
  }
  return c;
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(1);
  setup_fb();
  setup_kbd();
  send_ipc_call("logd", IPC_LOGD_REGISTER, 0, 0, 0, 0, 0);
  drop_capability(CAP_NAMESPACE_SERVERS, CAP_LOGD_TTY);
  setenv("PATH", "/sbin:/bin", 0);
  spawn_process("/bin/sh");
  start_process();
  ipc_handlers[IPC_VFSD_FS_WRITE] = print_handler;
  ipc_handlers[IPC_TTYD_KBD_INPUT] = kbd_input_handler;
  while (1) {
    handle_ipc();
  }
}

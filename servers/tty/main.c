#include <capability.h>
#include <ipc.h>
#include <spawn.h>
#include <stdlib.h>
#include <syslog.h>
#include <tty/fb.h>
#include <tty/kbd.h>

static int64_t print_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  if (arg0 || arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  size_t fb;
  if (is_caller_child()) {
    fb = 0;
  } else if (has_ipc_caller_capability(CAP_NAMESPACE_SERVERS, CAP_LOGD)) {
    fb = 11;
  } else {
    syslog(LOG_DEBUG, "No permission to print to screen");
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
  send_ipc_call("logd", 0, 0, 0, 0, 0, 0);
  drop_capability(CAP_NAMESPACE_SERVERS, CAP_LOGD_TTY);
  setenv("PATH", "/sbin:/bin", 0);
  spawn_process("/bin/sh");
  start_process();
  ipc_handlers[1] = kbd_input_handler;
  ipc_handlers[IPC_CALL_MEMORY_SHARING] = print_handler;
  while (1) {
    handle_ipc();
  }
}

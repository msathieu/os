#include <ipccalls.h>
#include <stdio.h>

__attribute__((weak)) extern int _disable_syslog;

void syslog(__attribute__((unused)) int priority, const char* format, ...) {
  if (&_disable_syslog) {
    return;
  }
  va_list args;
  va_start(args, format);
  char buf[512];
  size_t len = vsnprintf(buf, 512, format, args);
  va_end(args);
  send_ipc_call("logd", IPC_LOGD_LOG, 0, 0, 0, (uintptr_t) buf, len + 1);
}

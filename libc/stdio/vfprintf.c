#include <ipc.h>
#include <stdio.h>
#include <string.h>

int vfprintf(FILE* restrict file, const char* restrict format, va_list args) {
  char buf[512];
  int return_value = vsnprintf(buf, 512, format, args);
  if (file == stdout) {
    send_ipc_call("ttyd", IPC_CALL_MEMORY_SHARING, 0, 0, 0, (uintptr_t) buf, strlen(buf) + 1);
  } else {
    for (size_t i = 0; buf[i]; i++) {
      putc(buf[i], file);
    }
  }
  return return_value;
}

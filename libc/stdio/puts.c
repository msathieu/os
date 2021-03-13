#include <ipc.h>
#include <stdio.h>
#include <string.h>

int puts(const char* str) {
  send_ipc_call("ttyd", IPC_CALL_MEMORY_SHARING, 0, 0, 0, (uintptr_t) str, strlen(str) + 1);
  putchar('\n');
  return 0;
}

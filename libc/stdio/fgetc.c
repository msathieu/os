#include <ipccalls.h>
#include <stdio.h>

int fgetc(FILE* file) {
  if (file == stdin) {
    return send_ipc_call("ttyd", IPC_TTYD_KBD_INPUT, 0, 0, 0, 0, 0);
  }
  unsigned char c;
  size_t return_value = fread(&c, 1, 1, file);
  if (return_value) {
    return c;
  } else {
    return EOF;
  }
}

#include <ipc.h>
#include <stdio.h>
#include <stdlib.h>

int getc(FILE* file) {
  if (file == stdin) {
    return send_ipc_call("ttyd", 1, 0, 0, 0, 0, 0);
  }
  exit(1);
}

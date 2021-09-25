#include <ipccalls.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    puts("Invalid arguments");
    return 1;
  }
  if (strlen(argv[1]) != 2) {
    puts("Invalid keyboard layout");
    return 1;
  }
  send_ipc_call("kbdd", IPC_KBDD_CHANGE_LAYOUT, argv[1][0], argv[1][1], 0, 0, 0);
}

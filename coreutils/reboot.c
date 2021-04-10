#include <ipccalls.h>

int main(void) {
  send_ipc_call("acpid", IPC_ACPID_POWER_STATE, 1, 0, 0, 0, 0);
}

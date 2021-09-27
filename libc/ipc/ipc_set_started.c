#include <ipccalls.h>

void ipc_set_started(void) {
  send_pid_ipc_call(2, IPC_IPCD_SET_STARTED, 0, 0, 0, 0, 0);
}

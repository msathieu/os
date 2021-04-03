#include <ipccalls.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>

void register_ipc_name(const char* name) {
  char process_name[5];
  strncpy(process_name, name, 5);
  int64_t return_value = send_pid_ipc_call(2, IPC_IPCD_REGISTER, process_name[0], process_name[1], process_name[2], process_name[3], process_name[4]);
  if (return_value == -IPC_ERR_INVALID_PID) {
    sched_yield();
    return register_ipc_name(name);
  }
  if (return_value < 0) {
    exit(1);
  }
}

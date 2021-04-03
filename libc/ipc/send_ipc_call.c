#include <ipccalls.h>
#include <sched.h>
#include <string.h>

int64_t send_ipc_call(const char* name, uint8_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  static char process_name[5] = "";
  static int64_t pid = 0;
  if (strncmp(process_name, name, 5)) {
    strncpy(process_name, name, 5);
    pid = 0;
  }
  bool first_iteration = 1;
  while (1) {
    if (!first_iteration || !pid) {
      pid = send_pid_ipc_call(2, IPC_IPCD_DISCOVER, process_name[0], process_name[1], process_name[2], process_name[3], process_name[4]);
    }
    first_iteration = 0;
    if (pid > 0) {
      int64_t return_value = send_pid_ipc_call(pid, syscall, arg0, arg1, arg2, arg3, arg4);
      if (return_value != -IPC_ERR_INVALID_PID) {
        return return_value;
      }
    }
    sched_yield();
  }
}

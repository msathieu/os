#include <__/syscall.h>
#include <stdlib.h>
#include <string.h>

void register_ipc_name(const char* name) {
  char process_name[5];
  strncpy(process_name, name, 5);
  int64_t return_value = _syscall(_SYSCALL_REGISTER_IPC_NAME, process_name[0], process_name[1], process_name[2], process_name[3], process_name[4]);
  if (return_value < 0) {
    exit(1);
  }
}

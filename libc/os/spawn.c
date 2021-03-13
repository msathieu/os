#include <__/syscall.h>
#include <ipc.h>
#include <string.h>
#include <sys/types.h>

extern char** environ;
static bool has_arguments;

pid_t spawn_process_raw(const char* file) {
  has_arguments = 0;
  char os_file[5];
  strncpy(os_file, file, 5);
  return _syscall(_SYSCALL_SPAWN, os_file[0], os_file[1], os_file[2], os_file[3], os_file[4]);
}
void start_process(void) {
  for (size_t i = 0; environ[i]; i++) {
    send_ipc_call("envd", IPC_CALL_MEMORY_SHARING, 0, 0, 0, (uintptr_t) environ[i], strlen(environ[i]) + 1);
  }
  _syscall(_SYSCALL_START, has_arguments, (bool) environ[0], 0, 0, 0);
}
void grant_ioport(uint16_t port) {
  _syscall(_SYSCALL_GRANT_IOPORT, port, 0, 0, 0, 0);
}
void register_irq(int irq) {
  _syscall(_SYSCALL_REGISTER_IRQ, 0, irq, 0, 0, 0);
}
void grant_capability(int namespace, int capability) {
  _syscall(_SYSCALL_GRANT_CAPABILITIES, namespace, 1 << capability, 0, 0, 0);
}
void add_argument(const char* arg) {
  has_arguments = 1;
  send_ipc_call("argd", IPC_CALL_MEMORY_SHARING, 0, 0, 0, (uintptr_t) arg, strlen(arg) + 1);
}
pid_t spawn_process(const char* file) {
  pid_t pid = spawn_process_raw("lelf");
  add_argument(file);
  return pid;
}

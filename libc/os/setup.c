#include <__/rpmalloc.h>
#include <__/syscall.h>
#include <ipc.h>
#include <stdlib.h>

size_t _argc;
char** _argv;
char** environ;
__attribute__((weak)) extern int _noremove_args;
__attribute__((weak)) extern int _noenvironment_vars;
extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);

void _setup_libc(void) {
  rpmalloc_initialize();
  if (_syscall(_SYSCALL_HAS_ARGUMENTS, 0, 0, 0, 0, 0)) {
    _argc = send_ipc_call("argd", 0, 0, 0, 0, 0, 0);
    _argv = malloc((_argc + 1) * sizeof(char*));
    for (size_t i = 0; i < _argc; i++) {
      size_t size = send_ipc_call("argd", 1, i, 0, 0, 0, 0);
      _argv[i] = calloc(size, 1);
      bool noremove = 0;
      if (&_noremove_args) {
        noremove = 1;
      }
      send_ipc_call("argd", IPC_CALL_MEMORY_SHARING_RW, i, noremove, 0, (uintptr_t) _argv[i], size);
    }
    _argv[_argc] = 0;
  } else {
    _argv = calloc(1, sizeof(char*));
  }
  if (!&_noenvironment_vars && _syscall(_SYSCALL_HAS_ENVIRONMENT_VARS, 0, 0, 0, 0, 0)) {
    size_t nenvs = send_ipc_call("envd", 0, 0, 0, 0, 0, 0);
    environ = malloc((nenvs + 1) * sizeof(char*));
    for (size_t i = 0; i < nenvs; i++) {
      size_t size = send_ipc_call("envd", 1, i, 0, 0, 0, 0);
      environ[i] = calloc(size, 1);
      send_ipc_call("envd", IPC_CALL_MEMORY_SHARING_RW, i, 0, 0, (uintptr_t) environ[i], size);
    }
    environ[nenvs] = 0;
  } else {
    environ = calloc(1, sizeof(char*));
  }
  for (size_t i = 0; i < (size_t)(__init_array_end - __init_array_start); i++) {
    __init_array_start[i]();
  }
}

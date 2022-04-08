#include <__/rpmalloc.h>
#include <__/syscall.h>
#include <ipccalls.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <threads.h>
#include <unistd.h>

size_t _argc;
char** _argv;
static void* tls_master;
size_t _tls_size;
__attribute__((weak)) extern int _noremove_args;
__attribute__((weak)) extern int _noenvironment_vars;
extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);

static void setup_thread_libc(void* tls) {
  *((uintptr_t*) (tls + _tls_size)) = (uintptr_t) tls + _tls_size;
  memcpy(tls, tls_master, _tls_size);
  _syscall(_SYSCALL_SET_FS, (uintptr_t) tls + _tls_size, 0, 0, 0, 0);
  rpmalloc_initialize();
}
void _setup_libc(uintptr_t _tls_master, size_t tls_size) {
  tls_master = (void*) _tls_master;
  _tls_size = tls_size;
  void* initial_tls = mmap(0, tls_size + sizeof(struct _thread), PROT_WRITE, MAP_ANONYMOUS, 0, 0);
  setup_thread_libc(initial_tls);
  if (_syscall(_SYSCALL_HAS_ARGUMENTS, 0, 0, 0, 0, 0)) {
    _argc = send_ipc_call("argd", IPC_ARGD_GET_NUM, 0, 0, 0, 0, 0);
    _argv = malloc((_argc + 1) * sizeof(char*));
    for (size_t i = 0; i < _argc; i++) {
      size_t size = send_ipc_call("argd", IPC_ARGD_GET_SIZE, i, 0, 0, 0, 0);
      _argv[i] = calloc(size, 1);
      send_ipc_call("argd", IPC_ARGD_GET, i, (bool) &_noremove_args, 0, (uintptr_t) _argv[i], size);
    }
    _argv[_argc] = 0;
  } else {
    _argv = calloc(1, sizeof(char*));
  }
  if (!&_noenvironment_vars && _syscall(_SYSCALL_HAS_ARGUMENTS, 1, 0, 0, 0, 0)) {
    size_t nenvs = send_ipc_call("envd", IPC_ENVD_GET_NUM, 0, 0, 0, 0, 0);
    environ = malloc((nenvs + 1) * sizeof(char*));
    for (size_t i = 0; i < nenvs; i++) {
      size_t size = send_ipc_call("envd", IPC_ENVD_GET_SIZE, i, 0, 0, 0, 0);
      environ[i] = calloc(size, 1);
      send_ipc_call("envd", IPC_ENVD_GET, i, 0, 0, (uintptr_t) environ[i], size);
    }
    environ[nenvs] = 0;
  } else {
    environ = calloc(1, sizeof(char*));
  }
  for (size_t i = 0; i < (size_t) (__init_array_end - __init_array_start); i++) {
    __init_array_start[i]();
  }
}
int _thread_trampoline(thrd_t thread) {
  setup_thread_libc((void*) thread - _tls_size);
  return thread->func(thread->arg);
}

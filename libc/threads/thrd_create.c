#include <__/syscall.h>
#include <stdlib.h>
#include <threads.h>

extern size_t _tls_size;
extern void _thread_entry(void);

int thrd_create(thrd_t* ptr, thrd_start_t func, void* arg) {
  thrd_t thread = calloc(1, _tls_size + sizeof(struct _thread)) + _tls_size;
  thread->stack = malloc(0x8000);
  thread->tls_start = (void*) thread - _tls_size;
  thread->func = func;
  thread->arg = arg;
  _syscall(_SYSCALL_SPAWN_THREAD, (uintptr_t) _thread_entry, (uintptr_t) thread, (uintptr_t) thread->stack + 0x8000, 0, 0);
  if (ptr) {
    *ptr = thread;
  }
  return 0;
}

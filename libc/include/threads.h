#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef int (*thrd_start_t)(void*);
typedef struct _thread {
  uintptr_t self;
  void* stack;
  void* tls_start;
  thrd_start_t func;
  void* arg;
} * thrd_t;

int thrd_create(thrd_t* thread, thrd_start_t func, void* arg);
thrd_t thrd_current(void);
_Noreturn void thrd_exit(int status);

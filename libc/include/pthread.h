#pragma once
#include <threads.h>

typedef thrd_t pthread_t;
typedef bool pthread_spinlock_t;

_Noreturn void pthread_exit(void* value);
pthread_t pthread_self(void);
int pthread_spin_destroy(pthread_spinlock_t* spinlock);
int pthread_spin_init(pthread_spinlock_t* spinlock, int shared);
int pthread_spin_lock(pthread_spinlock_t* spinlock);
int pthread_spin_trylock(pthread_spinlock_t* spinlock);
int pthread_spin_unlock(pthread_spinlock_t* spinlock);

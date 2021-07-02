#pragma once
#include <sys/types.h>

pid_t get_exited_pid(void);
void listen_exits(void);
pid_t wait(int* status);

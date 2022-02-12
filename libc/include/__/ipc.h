#pragma once
#include <ipc.h>

struct ipc_call {
  ipc_handler handler;
  size_t nargs;
};

extern struct ipc_call _ipc_calls[256];

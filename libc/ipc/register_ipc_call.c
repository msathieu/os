#include <__/ipc.h>

void register_ipc_call(size_t syscall, ipc_handler handler, size_t nargs) {
  _ipc_calls[syscall].nargs = nargs;
  _ipc_calls[syscall].handler = handler;
}

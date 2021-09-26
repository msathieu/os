.global handle_ipc
handle_ipc:
  mov $8, %rdi
  xor %rsi, %rsi
  xor %rdx, %rdx
  xor %rcx, %rcx
  xor %r8, %r8
  xor %r9, %r9
  call _syscall
  call ipc_common
  xor %rdx, %rdx
  xor %rcx, %rcx
  xor %r8, %r8
  xor %r9, %r9
  mov $-2, %rdi
  cmp %rax, %rdi
  je block_call
  mov $9, %rdi
  mov %rax, %rsi
  call _syscall
  ret
block_call:
  mov $22, %rdi
  xor %rsi, %rsi
  call _syscall
  ret

.global _syscall
_syscall:
  mov %rcx, %rax
  syscall
  ret

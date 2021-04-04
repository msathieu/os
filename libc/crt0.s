.global _rdrand
_rdrand:
  rdrand %rax
  jnc _rdrand
  ret
.global _start
_start:
  mov $stack, %rsp
  call _rdrand
  mov %rax, __stack_chk_guard
  call _setup_libc
  mov _argc, %rdi
  mov _argv, %rsi
  call main
  mov %rax, %rdi
  call exit
.section .bss
.align 16
.skip 0x8000
stack:

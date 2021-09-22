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
.global _thread_entry
_thread_entry:
  mov %rsi, %rsp
  call _thread_trampoline
  mov %rax, %rdi
  call pthread_exit
.section .bss
.align 16
.skip 0x8000
stack:

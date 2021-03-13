.global syscall
syscall:
  swapgs
  mov %rsp, %gs:12
  mov %gs:4, %rsp
  push $0x1b
  push %gs:12
  swapgs
  push %r11
  push $0x23
  push %rcx
  push $0
  push $0
  push %r15
  push %r14
  push %r13
  push %r12
  push %r11
  push %r10
  push %r9
  push %r8
  push %rbp
  push %rdi
  push %rsi
  push %rdx
  push %rax
  push %rbx
  push %rax
  mov %rsp, %rdi

  xor %rax, %rax
  mov %rax, %ds
  mov %rax, %es
  mov %rax, %fs
  mov %rax, %gs
  call syscall_common

  jmp isr_return

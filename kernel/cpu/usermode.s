.global jmp_user
jmp_user:
  push $0x1b
  push $0
  pushf
  btsq $9, (%rsp)
  push $0x23
  push %rdx

  xor %rax, %rax
  xor %rbx, %rbx
  xor %rcx, %rcx
  xor %rdx, %rdx
  xor %rbp, %rbp
  xor %r8, %r8
  xor %r9, %r9
  xor %r10, %r10
  xor %r11, %r11
  xor %r12, %r12
  xor %r13, %r13
  xor %r14, %r14
  xor %r15, %r15

  fninit
  iretq

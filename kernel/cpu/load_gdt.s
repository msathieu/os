.global load_gdt
load_gdt:
  lgdt (%rdi)
  xor %rax, %rax
  mov %rax, %ds
  mov %rax, %es
  mov %rax, %fs
  mov %rax, %gs
  mov %rax, %ss
  mov $0x28, %rax
  ltr %ax
  ret

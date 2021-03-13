.global jmp_kernel
jmp_kernel:
  lgdt gdt_descriptor
  jmp $8, $x64_trampoline
.code64
x64_trampoline:
  mov $0x375f3b9858ea0482, %rdi
  mov loader_struct_ptr(%rip), %rsi
  jmp *kernel_entry(%rip)

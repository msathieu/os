.global _start
_start:
  mov $boot_stack, %rsp
  call rdrand
  mov %rax, __stack_chk_guard
  call kmain

.section .bss
.align 16
.skip 0x2000
.global boot_stack
boot_stack:
.global malloc_start
malloc_start:
.skip 0x800000

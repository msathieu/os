.section .multiboot
.align 4
multiboot_start:
.int 0xe85250d6
.int 0
.int multiboot_end - multiboot_start
.int -(0xe85250d6 + multiboot_end - multiboot_start)

.short 5
.short 0
.int 20
.int 0
.int 0
.int 0

.align 8
.short 0
.short 0
.int 8
multiboot_end:

.section .text
.global start
start:
  mov $stack, %esp
  push %ebx
  push %eax
  call lmain

.section .bss
.align 16
.skip 0x2000
stack:
.global malloc_start
malloc_start:
.skip 0x80000

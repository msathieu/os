.section .rodata
.code16
.align 0x1000

.global ap_trampoline
ap_trampoline:
  jmp trampoline_2

.align 0x10
stack: .quad 0
pml4: .int 0

gdt:
.quad 0
.quad 0x209a0000000000
.quad 0x920000000000
gdt_desc:
.short . - gdt - 1
.int gdt - ap_trampoline + 0x1000

trampoline_2:
  mov %cr4, %eax
  bts $5, %eax // Physical address extension
  mov %eax, %cr4
  mov $0xc0000080, %ecx
  rdmsr
  bts $8, %eax // Long mode enable
  bts $11, %eax // Execute disable
  wrmsr
  mov pml4 - ap_trampoline + 0x1000, %eax
  mov %eax, %cr3
  mov %cr0, %eax
  bts $0, %eax // Protected mode enable
  bts $31, %eax // Paging
  mov %eax, %cr0
  lgdt gdt_desc - ap_trampoline + 0x1000
  ljmp $8, $0x1100

.code64
.align 0x100
  mov stack - ap_trampoline + 0x1000, %rsp
  mov $ap_entry, %rax
  call *%rax

#include <cpu/gdt.h>
#include <cpu/kernel.h>
#include <cpu/paging.h>
#include <cpuid.h>
#include <multiboot.h>
#include <panic.h>
#include <struct.h>

struct loader_struct loader_struct = {.version = 0};
uint64_t loader_struct_ptr;

void lmain(int magic, uintptr_t mbptr) {
  unsigned eax, ebx, ecx, edx;
  if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
    panic("Error obtaining CPUID information");
  }
  if (ecx & 0x80000000) {
    loader_struct.debug_port = 0x3f8;
  }
  if (magic != 0x36d76289) {
    panic("Magic value is incorrect");
  }
  if (!(ecx & 0x40000000)) {
    panic("rdrand isn't supported");
  }
  if (!__get_cpuid(0x80000001, &eax, &ebx, &ecx, &edx)) {
    panic("CPU doesn't support x86_64");
  }
  if (!(edx & 0x20000000)) {
    panic("CPU doesn't support x86_64");
  }
  parse_multiboot_header(mbptr);
  setup_paging();
  setup_gdt();
  loader_struct_ptr = (uintptr_t) &loader_struct;
  jmp_kernel();
}

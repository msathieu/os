#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpuid.h>
#include <panic.h>
#include <stdint.h>
#include <sys/lapic.h>

volatile bool ap_startup;
size_t ap_nlapic;
bool smp_lock;
extern void syscall(void);

void set_cpu_flags(void) {
  unsigned eax, ebx, ecx = 0, edx = 0;
  __get_cpuid(1, &eax, &ebx, &ecx, &edx);
  if (edx & 0x10) {
    asm volatile("mov %%cr4, %%rax; bts $2, %%rax; mov %%rax, %%cr4" // Time stamp disable
                 :
                 :
                 : "rax");
  }
  __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx);
  if (ecx & 4) {
    asm volatile("mov %%cr4, %%rax; bts $11, %%rax; mov %%rax, %%cr4" // User-mode instruction prevention
                 :
                 :
                 : "rax");
  }
  __get_cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
  if (!(edx & 0x800)) {
    panic("SYSCALL isn't supported");
  }
  asm volatile("rdmsr; bts $0, %%rax; wrmsr"
               :
               : "c"(0xc0000080)
               : "rax", "rdx");
  asm volatile("rdmsr; mov $8, %%rdx; wrmsr"
               :
               : "c"(0xc0000081)
               : "rax", "rdx");
  asm volatile("wrmsr"
               :
               : "c"(0xc0000082), "a"(syscall), "d"((uintptr_t) syscall >> 32));
  asm volatile("rdmsr; bts $9, %%rax; bts $10, %%rax; wrmsr"
               :
               : "c"(0xc0000084)
               : "rax", "rdx");
  asm volatile("mov %%cr0, %%rax; bts $5, %%rax; mov %%rax, %%cr0" // Numeric error
               :
               :
               : "rax");
  asm volatile("mov %%cr4, %%rax; bts $9, %%rax; bts $10, %%rax; mov %%rax, %%cr4" // SSE and SSE exceptions
               :
               :
               : "rax");
  asm volatile("mov %%cr0, %%rax; bts $16, %%rax; mov %%rax, %%cr0" // Write protect
               :
               :
               : "rax");
  asm volatile("mov %%cr4, %%rax; bts $7, %%rax; mov %%rax, %%cr4" // Page global enable
               :
               :
               : "rax");
  __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx);
  if (ebx & 0x80) {
    asm volatile("mov %%cr4, %%rax; bts $20, %%rax; mov %%rax, %%cr4" // Supervisor-mode execution prevention
                 :
                 :
                 : "rax");
  }
  if (ebx & 0x100000) {
    asm volatile("mov %%cr4, %%rax; bts $21, %%rax; mov %%rax, %%cr4" // Supervisor-mode access prevention
                 :
                 :
                 : "rax");
  }
}

void ap_entry(void) {
  setup_gdt(1);
  load_idt();
  set_cpu_flags();
  setup_lapic(ap_nlapic);
  setup_lapic_timer(1);
  ap_startup = 1;
  while (1) {
    asm volatile("hlt");
  }
}

#pragma once
#include <stdint.h>

struct isr_registers {
  uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
  uint64_t isr, error;
  uint64_t rip, cs, rflags, rsp, ss;
};

typedef void (*isr_handler)(struct isr_registers*);

extern isr_handler isr_handlers[256];

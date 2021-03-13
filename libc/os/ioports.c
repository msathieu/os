#include <__/syscall.h>

uint8_t inb(uint16_t port) {
  return _syscall(_SYSCALL_ACCESS_IOPORT, port, 0, 0, 0, 0);
}
void outb(uint16_t port, uint8_t value) {
  _syscall(_SYSCALL_ACCESS_IOPORT, port, 1, 0, value, 0);
}
uint16_t inw(uint16_t port) {
  return _syscall(_SYSCALL_ACCESS_IOPORT, port, 0, 1, 0, 0);
}
void outw(uint16_t port, uint16_t value) {
  _syscall(_SYSCALL_ACCESS_IOPORT, port, 1, 1, value, 0);
}
uint32_t inl(uint16_t port) {
  return _syscall(_SYSCALL_ACCESS_IOPORT, port, 0, 2, 0, 0);
}
void outl(uint16_t port, uint32_t value) {
  _syscall(_SYSCALL_ACCESS_IOPORT, port, 1, 2, value, 0);
}

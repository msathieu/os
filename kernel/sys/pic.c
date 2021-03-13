#include <cpu/ioports.h>
#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xa0
#define PIC2_DATA 0xa1

void disable_pic(void) {
  outb(PIC1_CMD, 0x11);
  outb(PIC2_CMD, 0x11);
  outb(PIC1_DATA, 0x20);
  outb(PIC2_DATA, 0x28);
  outb(PIC1_DATA, 4);
  outb(PIC2_DATA, 2);
  outb(PIC1_DATA, 1);
  outb(PIC2_DATA, 1);
  outb(PIC1_DATA, 0xff);
  outb(PIC2_DATA, 0xff);
}

#include <panic.h>

unsigned long __udivdi3(__attribute__((unused)) unsigned long a, __attribute__((unused)) unsigned long b) {
  panic("Function shouldn't be called");
}

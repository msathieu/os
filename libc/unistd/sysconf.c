#include <unistd.h>

long sysconf(int name) {
  switch (name) {
  case _SC_PAGESIZE:
    return 0x1000;
  default:
    return -1;
  }
}

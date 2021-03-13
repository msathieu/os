#include <stddef.h>

int posix_madvise(__attribute__((unused)) void* address, __attribute__((unused)) size_t size, __attribute__((unused)) int advice) {
  return 0;
}

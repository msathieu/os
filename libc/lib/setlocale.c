#include <string.h>

char* setlocale(__attribute__((unused)) int category, const char* locale) {
  if (!locale || !strcmp(locale, "C")) {
    return "C";
  } else {
    return 0;
  }
}

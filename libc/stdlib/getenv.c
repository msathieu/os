#include <string.h>

extern char** environ;

char* getenv(const char* name) {
  for (size_t i = 0; environ[i]; i++) {
    char* value = strchr(environ[i], '=');
    if ((size_t)(value - environ[i]) == strlen(name) && !memcmp(environ[i], name, value - environ[i])) {
      return value + 1;
    }
  }
  return 0;
}

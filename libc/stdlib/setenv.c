#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern char** environ;

int setenv(const char* name, const char* value, int overwrite) {
  if (strchr(name, '=')) {
    errno = EINVAL;
    return -1;
  }
  size_t i;
  for (i = 0; environ[i]; i++) {
    char* env_value = strchr(environ[i], '=');
    if ((size_t) (env_value - environ[i]) == strlen(name) && !memcmp(environ[i], name, env_value - environ[i])) {
      if (overwrite) {
        free(environ[i]);
        environ[i] = malloc(env_value - environ[i] + 1 + strlen(value) + 1);
        strcpy(environ[i], name);
        strcat(environ[i], "=");
        strcat(environ[i], value);
      }
      return 0;
    }
  }
  environ = realloc(environ, (i + 2) * sizeof(char*));
  environ[i + 1] = 0;
  environ[i] = malloc(strlen(name) + 1 + strlen(value) + 1);
  strcpy(environ[i], name);
  strcat(environ[i], "=");
  strcat(environ[i], value);
  return 0;
}

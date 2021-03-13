#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

extern char** environ;

static char* read_line(void) {
  char* buffer = malloc(512);
  size_t bufsize = 512;
  size_t len = 0;
  while (1) {
    buffer[len] = getchar();
    if (buffer[len] == '\n') {
      buffer[len] = 0;
      return buffer;
    }
    len++;
    if (len == bufsize) {
      bufsize += 512;
      buffer = realloc(buffer, bufsize);
    }
  }
}
char** parse_line(char* buffer) {
  char** args = calloc(1, sizeof(char*));
  size_t i = 0;
  for (char* arg = strtok(buffer, " "); arg; arg = strtok(0, " ")) {
    i++;
    args = realloc(args, (i + 1) * sizeof(char*));
    args[i - 1] = strdup(arg);
    args[i] = 0;
  }
  return args;
}
static void builtin_exit(char* return_value) {
  if (return_value) {
    exit(atoi(return_value));
  } else {
    exit(0);
  }
}
static void builtin_export(char* variable) {
  if (!variable) {
    return;
  }
  if (!strcmp(variable, "-p")) {
    for (size_t i = 0; environ[i]; i++) {
      printf("export %s\n", environ[i]);
    }
  } else {
    char* value = strchr(variable, '=');
    if (!value) {
      return;
    }
    value[0] = 0;
    setenv(variable, value + 1, 1);
  }
}
static bool match_builtin(char** args) {
  if (!strcmp(args[0], "exit")) {
    builtin_exit(args[1]);
    return 1;
  } else if (!strcmp(args[0], "export")) {
    builtin_export(args[1]);
    return 1;
  }
  return 0;
}
static char* expand_path(const char* program) {
  const char* const_path = getenv("PATH");
  if (!const_path || !const_path[0]) {
    return 0;
  }
  char* path_env = strdup(const_path);
  char* path = 0;
  for (char* path_env_i = strtok(path_env, ":"); path_env_i; path_env_i = strtok(0, ":")) {
    char* path_i = malloc(strlen(path_env_i) + 1 + strlen(program) + 1);
    strcpy(path_i, path_env_i);
    strcat(path_i, "/");
    strcat(path_i, program);
    FILE* file = fopen(path_i, "r");
    if (file) {
      fclose(file);
      path = path_i;
      break;
    }
    free(path_i);
  }
  free(path_env);
  return path;
}
static void execute_line(char** args) {
  if (!args[0]) {
    return;
  }
  if (match_builtin(args)) {
    return;
  }
  if (strchr(args[0], '/')) {
    FILE* file = fopen(args[0], "r");
    if (file) {
      fclose(file);
      spawn_process(args[0]);
    } else {
      printf("Command not found: %s\n", args[0]);
      return;
    }
  } else {
    char* path = expand_path(args[0]);
    if (path) {
      spawn_process(path);
      free(path);
    } else {
      printf("Command not found: %s\n", args[0]);
      return;
    }
  }
  for (size_t i = 1; args[i]; i++) {
    add_argument(args[i]);
  }
  start_process();
  wait(0);
}
int main(void) {
  while (1) {
    printf("$ ");
    char* buffer = read_line();
    char** args = parse_line(buffer);
    free(buffer);
    execute_line(args);
    for (size_t i = 0; args[i]; i++) {
      free(args[i]);
    }
    free(args);
  }
}

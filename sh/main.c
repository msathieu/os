#include <builtins.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

enum {
  TYPE_NOP,
  TYPE_BUILTIN,
  TYPE_PROGRAM
};
struct parsed_line {
  int type;
  builtin_t builtin;
  char* file;
  char* args[64];
  char* stdout;
};

static char* read_line(void) {
  char* buffer = malloc(512);
  size_t bufsize = 512;
  size_t len = 0;
  while (true) {
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
static char** split_line(char* buffer) {
  char** tokens = calloc(1, sizeof(char*));
  size_t i = 0;
  for (char* token = strtok(buffer, " \t"); token; token = strtok(0, " \t")) {
    i++;
    tokens = realloc(tokens, (i + 1) * sizeof(char*));
    tokens[i - 1] = strdup(token);
    tokens[i] = 0;
  }
  return tokens;
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
static struct parsed_line* parse_line(char** tokens) {
  struct parsed_line* parsed_line = calloc(1, sizeof(struct parsed_line));
  if (!tokens[0]) {
    parsed_line->type = TYPE_NOP;
    return parsed_line;
  }
  parsed_line->type = TYPE_BUILTIN;
  if (!strcmp(tokens[0], "exit")) {
    parsed_line->builtin = builtin_exit;
  } else if (!strcmp(tokens[0], "export")) {
    parsed_line->builtin = builtin_export;
  } else {
    parsed_line->type = TYPE_PROGRAM;
    if (strchr(tokens[0], '/')) {
      FILE* file = fopen(tokens[0], "r");
      if (file) {
        fclose(file);
        parsed_line->file = strdup(tokens[0]);
      } else {
        printf("Command not found: %s\n", tokens[0]);
        parsed_line->type = TYPE_NOP;
        return parsed_line;
      }
    } else {
      char* path = expand_path(tokens[0]);
      if (path) {
        parsed_line->file = path;
      } else {
        printf("Command not found: %s\n", tokens[0]);
        parsed_line->type = TYPE_NOP;
        return parsed_line;
      }
    }
  }
  size_t arg_i = 0;
  for (size_t i = 1; tokens[i]; i++) {
    if (!strcmp(tokens[i], ">")) {
      if (!tokens[i + 1]) {
        puts("Missing stdout path");
        parsed_line->type = TYPE_NOP;
        return parsed_line;
      }
      parsed_line->stdout = strdup(tokens[i + 1]);
      break;
    } else {
      if (arg_i > 63) {
        puts("Too many arguments");
        parsed_line->type = TYPE_NOP;
        return parsed_line;
      }
      parsed_line->args[arg_i++] = strdup(tokens[i]);
    }
  }
  return parsed_line;
}
static void execute_line(struct parsed_line* parsed_line) {
  if (parsed_line->type == TYPE_BUILTIN) {
    FILE* _stdout = stdout;
    if (parsed_line->stdout) {
      _stdout = fopen(parsed_line->stdout, "w");
      if (!_stdout) {
        puts("Invalid stdout path");
        return;
      }
    }
    parsed_line->builtin(parsed_line->args[0], _stdout);
    if (parsed_line->stdout) {
      fclose(_stdout);
    }
  } else {
    if (parsed_line->stdout) {
      pid_t fork_pid = fork();
      if (fork_pid) {
        wait(0);
        return;
      }
      fclose(stdout);
      fopen(parsed_line->stdout, "w");
    }
    spawn_process(parsed_line->file);
    for (size_t i = 0; i < 63; i++) {
      if (parsed_line->args[i]) {
        add_argument(parsed_line->args[i]);
      }
    }
    start_process();
    wait(0);
    if (parsed_line->stdout) {
      exit(0);
    }
  }
}
int main(void) {
  while (true) {
    if (getuid()) {
      printf("$ ");
    } else {
      printf("# ");
    }
    char* buffer = read_line();
    char** tokens = split_line(buffer);
    free(buffer);
    struct parsed_line* parsed_line = parse_line(tokens);
    for (size_t i = 0; tokens[i]; i++) {
      free(tokens[i]);
    }
    free(tokens);
    if (parsed_line->type != TYPE_NOP) {
      execute_line(parsed_line);
    }
    if (parsed_line->file) {
      free(parsed_line->file);
    }
    if (parsed_line->stdout) {
      free(parsed_line->stdout);
    }
    for (size_t i = 0; i < 64; i++) {
      if (parsed_line->args[i]) {
        free(parsed_line->args[i]);
      }
    }
    free(parsed_line);
  }
}

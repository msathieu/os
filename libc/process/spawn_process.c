#include <spawn.h>

pid_t spawn_process(const char* file) {
  pid_t pid = spawn_process_raw("lelf");
  add_argument(file);
  return pid;
}

#include <__/syscall.h>
#include <capability.h>
#include <ipc.h>
#include <priority.h>
#include <spawn.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct process {
  char* name;
  bool raw;
  pid_t pid;
};

static struct process processes[] = {{"argd", 1, 0}, {"atad", 1, 0}, {"ipcd", 1, 0}, {"logd", 1, 0}, {"/sbin/envd", 0, 0}, {"/sbin/fbd", 0, 0}, {"/sbin/kbdd", 0, 0}, {"/sbin/ps2d", 0, 0}, {"/sbin/ttyd", 0, 0}, {"vfsd", 1, 0}};

static void spawn(const char* name) {
  for (size_t i = 0; i < sizeof(processes) / sizeof(struct process); i++) {
    if (!strcmp(processes[i].name, name)) {
      if (processes[i].raw) {
        processes[i].pid = spawn_process_raw(name);
      } else {
        processes[i].pid = spawn_process(name);
      }
      break;
    }
  }
  if (!strcmp(name, "argd")) {
    register_ipc_name("argd");
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  } else if (!strcmp(name, "atad")) {
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
    for (size_t i = 0; i < 8; i++) {
      grant_ioport(0x1f0 + i);
      grant_ioport(0x170 + i);
    }
    grant_ioport(0x3f6);
    grant_ioport(0x376);
    register_irq(14);
    register_irq(15);
    grant_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD_MOUNT);
  } else if (!strcmp(name, "ipcd")) {
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  } else if (!strcmp(name, "logd")) {
    register_ipc_name("logd");
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
    grant_capability(CAP_NAMESPACE_SERVERS, CAP_LOGD);
  } else if (!strcmp(name, "/sbin/envd")) {
    register_ipc_name("envd");
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  } else if (!strcmp(name, "/sbin/fbd")) {
    register_ipc_name("fbd");
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_GET_FB_INFO);
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
    uintptr_t fb_phys_addr = _syscall(_SYSCALL_GET_FB_INFO, 0, 0, 0, 0, 0);
    size_t height = _syscall(_SYSCALL_GET_FB_INFO, 2, 0, 0, 0, 0);
    size_t pitch = _syscall(_SYSCALL_GET_FB_INFO, 3, 0, 0, 0, 0);
    map_physical_memory(fb_phys_addr, height * pitch);
  } else if (!strcmp(name, "/sbin/kbdd")) {
    register_ipc_name("kbdd");
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
    grant_capability(CAP_NAMESPACE_SERVERS, CAP_KBDD);
  } else if (!strcmp(name, "/sbin/ps2d")) {
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
    grant_capability(CAP_NAMESPACE_SERVERS, CAP_KBDD_SEND_KEYPRESS);
    grant_ioport(0x60);
    grant_ioport(0x64);
    register_irq(1);
  } else if (!strcmp(name, "/sbin/ttyd")) {
    register_ipc_name("ttyd");
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
    grant_capability(CAP_NAMESPACE_DRIVERS, CAP_FBD_DRAW);
    grant_capability(CAP_NAMESPACE_SERVERS, CAP_KBDD_RECEIVE_EVENTS);
    grant_capability(CAP_NAMESPACE_SERVERS, CAP_LOGD_TTY);
  } else if (!strcmp(name, "vfsd")) {
    register_ipc_name("vfsd");
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
    grant_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD);
  }
  start_process();
}
int main(void) {
  if (getpid() != 1) {
    return 1;
  }
  spawn("ipcd");
  spawn("logd");
  spawn("argd");
  spawn("vfsd");
  spawn("atad");
  spawn("/sbin/envd");
  spawn("/sbin/fbd");
  spawn("/sbin/kbdd");
  spawn("/sbin/ttyd");
  spawn("/sbin/ps2d");
  while (1) {
    pid_t pid = wait(0);
    for (size_t i = 0; i < sizeof(processes) / sizeof(struct process); i++) {
      if (processes[i].pid == pid) {
        if (!strcmp(processes[i].name, "ipcd")) {
          return 1;
        }
        spawn(processes[i].name);
        break;
      }
    }
  }
}

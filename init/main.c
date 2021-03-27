#include <__/syscall.h>
#include <capability.h>
#include <ipc.h>
#include <priority.h>
#include <spawn.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct service {
  char* name;
  bool raw;
  pid_t pid;
  size_t capabilities[64];
};

struct service services[] = {
  {"argd", 1, 0, {[CAP_NAMESPACE_KERNEL] = 1 << CAP_KERNEL_PRIORITY}},
  {"atad", 1, 0, {[CAP_NAMESPACE_KERNEL] = 1 << CAP_KERNEL_PRIORITY, [CAP_NAMESPACE_FILESYSTEMS] = 1 << CAP_VFSD_MOUNT}},
  {"ipcd", 1, 0, {[CAP_NAMESPACE_KERNEL] = 1 << CAP_KERNEL_PRIORITY}},
  {"logd", 1, 0, {[CAP_NAMESPACE_KERNEL] = 1 << CAP_KERNEL_PRIORITY, [CAP_NAMESPACE_SERVERS] = 1 << CAP_LOGD}},
  {"/sbin/envd", 0, 0, {[CAP_NAMESPACE_KERNEL] = 1 << CAP_KERNEL_PRIORITY}},
  {"/sbin/fbd", 0, 0, {[CAP_NAMESPACE_KERNEL] = 1 << CAP_KERNEL_PRIORITY | 1 << CAP_KERNEL_GET_FB_INFO}},
  {"/sbin/kbdd", 0, 0, {[CAP_NAMESPACE_KERNEL] = 1 << CAP_KERNEL_PRIORITY, [CAP_NAMESPACE_SERVERS] = 1 << CAP_KBDD}},
  {"/sbin/ps2d", 0, 0, {[CAP_NAMESPACE_KERNEL] = 1 << CAP_KERNEL_PRIORITY, [CAP_NAMESPACE_SERVERS] = 1 << CAP_KBDD_SEND_KEYPRESS}},
  {"/sbin/ttyd", 0, 0, {[CAP_NAMESPACE_KERNEL] = 1 << CAP_KERNEL_PRIORITY, [CAP_NAMESPACE_DRIVERS] = 1 << CAP_FBD_DRAW, [CAP_NAMESPACE_SERVERS] = 1 << CAP_KBDD_RECEIVE_EVENTS | 1 << CAP_LOGD_TTY}},
  {"vfsd", 1, 0, {[CAP_NAMESPACE_KERNEL] = 1 << CAP_KERNEL_PRIORITY, [CAP_NAMESPACE_FILESYSTEMS] = 1 << CAP_VFSD}},
};

static void spawn(const char* name) {
  bool service_found = 0;
  for (size_t i = 0; i < sizeof(services) / sizeof(struct service); i++) {
    if (!strcmp(services[i].name, name)) {
      if (services[i].raw) {
        services[i].pid = spawn_process_raw(name);
      } else {
        services[i].pid = spawn_process(name);
      }
      for (size_t j = 0; j < 64; j++) {
        _syscall(_SYSCALL_GRANT_CAPABILITIES, j, services[i].capabilities[j], 0, 0, 0);
      }
      service_found = 1;
      break;
    }
  }
  if (!service_found) {
    return;
  }
  if (!strcmp(name, "argd")) {
    register_ipc_name("argd");
  } else if (!strcmp(name, "atad")) {
    for (size_t i = 0; i < 8; i++) {
      grant_ioport(0x1f0 + i);
      grant_ioport(0x170 + i);
    }
    grant_ioport(0x3f6);
    grant_ioport(0x376);
    register_irq(14);
    register_irq(15);
  } else if (!strcmp(name, "logd")) {
    register_ipc_name("logd");
  } else if (!strcmp(name, "/sbin/envd")) {
    register_ipc_name("envd");
  } else if (!strcmp(name, "/sbin/fbd")) {
    register_ipc_name("fbd");
    uintptr_t fb_phys_addr = _syscall(_SYSCALL_GET_FB_INFO, 0, 0, 0, 0, 0);
    size_t height = _syscall(_SYSCALL_GET_FB_INFO, 2, 0, 0, 0, 0);
    size_t pitch = _syscall(_SYSCALL_GET_FB_INFO, 3, 0, 0, 0, 0);
    map_physical_memory(fb_phys_addr, height * pitch);
  } else if (!strcmp(name, "/sbin/kbdd")) {
    register_ipc_name("kbdd");
  } else if (!strcmp(name, "/sbin/ps2d")) {
    grant_ioport(0x60);
    grant_ioport(0x64);
    register_irq(1);
  } else if (!strcmp(name, "/sbin/ttyd")) {
    register_ipc_name("ttyd");
  } else if (!strcmp(name, "vfsd")) {
    register_ipc_name("vfsd");
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
    for (size_t i = 0; i < sizeof(services) / sizeof(struct service); i++) {
      if (services[i].pid == pid) {
        if (!strcmp(services[i].name, "ipcd")) {
          return 1;
        }
        spawn(services[i].name);
        break;
      }
    }
  }
}

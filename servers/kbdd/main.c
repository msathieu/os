#include <capability.h>
#include <ipccalls.h>
#include <kbd/layout.h>
#include <keyboard.h>
#include <string.h>
#include <syslog.h>

static const struct layout* layouts[] = {&be_layout};
static const struct layout* layout;
static int toggles = 1 << KBD_TOGGLE_NUM_LOCK;
static int modifiers;
static pid_t event_receiver;

static int handle_key_event(struct layout_key* key, bool release) {
  if (release) {
    modifiers &= ~key->modifier;
  } else {
    modifiers |= key->modifier;
    if (toggles & key->toggle) {
      toggles &= ~key->toggle;
    } else {
      toggles |= key->toggle;
    }
  }
  int leds = 0;
  if (toggles & 1 << KBD_TOGGLE_CAPS_LOCK) {
    leds |= 1 << KBD_LED_CAPS_LOCK;
  }
  if (toggles & 1 << KBD_TOGGLE_NUM_LOCK) {
    leds |= 1 << KBD_LED_NUM_LOCK;
  }
  if (event_receiver && key->value) {
    send_pid_ipc_call(event_receiver, IPC_TTYD_KEY_EVENT, key->type, key->value, release, 0, 0);
  }
  return leds;
}
static int64_t keypress_handler(uint64_t keycode, uint64_t release, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg2 || arg3 || arg4) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (!has_ipc_caller_capability(CAP_NAMESPACE_SERVERS, CAP_KBDD_SEND_KEYPRESS)) {
    syslog(LOG_DEBUG, "No permission to send keypress");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  if (keycode >= 128) {
    syslog(LOG_DEBUG, "Invalid keycode");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (release >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  for (size_t i = 0; i < 16; i++) {
    struct layout_key key = layout->keys[layout->combinations[modifiers | toggles].parts[i]][keycode];
    if (key.value) {
      return handle_key_event(&key, release);
    }
  }
  struct layout_key placeholder_key = {0};
  return handle_key_event(&placeholder_key, release);
}
static int64_t registration_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg0 || arg1 || arg2 || arg3 || arg4) {
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (!has_ipc_caller_capability(CAP_NAMESPACE_SERVERS, CAP_KBDD_RECEIVE_EVENTS)) {
    syslog(LOG_DEBUG, "No permission to register keypress handler");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  event_receiver = get_ipc_caller_pid();
  return 0;
}
static int64_t change_layout_handler(uint64_t country0, uint64_t country1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg2 || arg3 || arg4) {
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (get_ipc_caller_uid()) {
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  char country[] = {country0, country1, 0};
  for (size_t i = 0; i < sizeof(layouts) / sizeof(uintptr_t); i++) {
    if (!strcmp(layouts[i]->country, country)) {
      layout = layouts[i];
      return 0;
    }
  }
  return -IPC_ERR_INVALID_ARGUMENTS;
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(0);
  layout = layouts[0];
  ipc_handlers[IPC_KBDD_KEYPRESS] = keypress_handler;
  ipc_handlers[IPC_KBDD_REGISTER] = registration_handler;
  ipc_handlers[IPC_KBDD_CHANGE_LAYOUT] = change_layout_handler;
  while (1) {
    handle_ipc();
  }
}

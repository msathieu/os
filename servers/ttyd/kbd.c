#include <capability.h>
#include <ipccalls.h>
#include <keyboard.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <tty/fb.h>

static char* buffer;
static size_t buffer_size;
static size_t buffer_len;
static bool ctrl_pressed;
static bool alt_pressed;
bool call_blocked;
size_t selected_framebuffer;

static int64_t key_event_handler(uint64_t type, uint64_t value, uint64_t release, __attribute__((unused)) uint64_t arg3, __attribute__((unused)) uint64_t arg4) {
  if (!has_ipc_caller_capability(CAP_NAMESPACE_SERVERS, CAP_KBDD)) {
    syslog(LOG_DEBUG, "No permission to send keypress");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  if (type >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (release >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (type == KBD_SPECIAL) {
    switch (value) {
    case KBD_CONTROL:
      ctrl_pressed = !release;
      return 0;
    case KBD_ALT:
      alt_pressed = !release;
      return 0;
    }
    if (ctrl_pressed && alt_pressed) {
      switch (value) {
      case KBD_F1:
        switch_framebuffer(0);
        return 0;
      case KBD_F2:
        switch_framebuffer(1);
        return 0;
      case KBD_F3:
        switch_framebuffer(2);
        return 0;
      case KBD_F4:
        switch_framebuffer(3);
        return 0;
      case KBD_F5:
        switch_framebuffer(4);
        return 0;
      case KBD_F6:
        switch_framebuffer(5);
        return 0;
      case KBD_F7:
        switch_framebuffer(6);
        return 0;
      case KBD_F8:
        switch_framebuffer(7);
        return 0;
      case KBD_F9:
        switch_framebuffer(8);
        return 0;
      case KBD_F10:
        switch_framebuffer(9);
        return 0;
      case KBD_F11:
        switch_framebuffer(10);
        return 0;
      case KBD_F12:
        switch_framebuffer(11);
        return 0;
      }
    }
  }
  if (!selected_framebuffer && !release) {
    if (type == KBD_CHARACTER) {
      put_character(value, 0);
      if (buffer_len == buffer_size) {
        buffer_size += 512;
        if (buffer) {
          buffer = realloc(buffer, buffer_size);
        } else {
          buffer = malloc(buffer_size);
        }
      }
      buffer[buffer_len++] = value;
      if (call_blocked && value == '\n') {
        ipc_unblock(0);
        call_blocked = false;
      }
    } else if (type == KBD_SPECIAL && value == KBD_BACKSPACE && buffer_len) {
      size_t nchars = 1;
      buffer_len--;
      if (buffer[buffer_len] == '\t') {
        nchars = 8;
      }
      for (size_t i = 0; i < nchars; i++) {
        fb_backspace(0);
      }
    }
  }
  update_fb();
  return 0;
}
char get_character(void) {
  if (buffer && memchr(buffer, '\n', buffer_len)) {
    char c = buffer[0];
    buffer_len--;
    memcpy(buffer, buffer + 1, buffer_len);
    return c;
  }
  return 0;
}
void setup_kbd(void) {
  send_ipc_call("kbdd", IPC_KBDD_REGISTER, 0, 0, 0, 0, 0);
  drop_capability(CAP_NAMESPACE_SERVERS, CAP_KBDD_RECEIVE_EVENTS);
  register_ipc_call(IPC_TTYD_KEY_EVENT, key_event_handler, 3);
}

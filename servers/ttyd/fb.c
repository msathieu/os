#include <capability.h>
#include <ipccalls.h>
#include <stdlib.h>
#include <string.h>
#include <tty/font.h>
#include <tty/kbd.h>

static size_t width;
static size_t height;
static size_t fb_width;
static size_t fb_height;
static size_t fb_pitch;
static size_t fb_bits_per_pixel;
static int fb_red_index;
static int fb_green_index;
static int fb_blue_index;
static size_t current_x[12];
static size_t current_y[12];
static char* framebuffer[12];
static bool* line_updated;
static bool screen_updated;
static bool cursor_drawn[12];

void setup_fb(void) {
  fb_width = send_ipc_call("fbd", IPC_FBD_INFO, 0, 0, 0, 0, 0);
  width = fb_width / 9;
  fb_height = send_ipc_call("fbd", IPC_FBD_INFO, 1, 0, 0, 0, 0);
  height = fb_height / 16;
  fb_pitch = send_ipc_call("fbd", IPC_FBD_INFO, 2, 0, 0, 0, 0);
  fb_bits_per_pixel = send_ipc_call("fbd", IPC_FBD_INFO, 3, 0, 0, 0, 0);
  fb_red_index = send_ipc_call("fbd", IPC_FBD_INFO, 4, 0, 0, 0, 0);
  fb_green_index = send_ipc_call("fbd", IPC_FBD_INFO, 5, 0, 0, 0, 0);
  fb_blue_index = send_ipc_call("fbd", IPC_FBD_INFO, 6, 0, 0, 0, 0);
  for (size_t i = 0; i < 12; i++) {
    framebuffer[i] = calloc(1, fb_height * fb_pitch);
  }
  line_updated = calloc(height, 1);
}
static void draw_pixel(size_t x, size_t y, int color, size_t fb) {
  framebuffer[fb][y * fb_pitch + x * fb_bits_per_pixel / 8 + fb_blue_index] = color;
  framebuffer[fb][y * fb_pitch + x * fb_bits_per_pixel / 8 + fb_green_index] = color >> 8;
  framebuffer[fb][y * fb_pitch + x * fb_bits_per_pixel / 8 + fb_red_index] = color >> 16;
}
static void draw_cursor(size_t x, size_t y, size_t fb, bool cursor) {
  if (fb == 11) {
    return;
  }
  for (size_t cy = 0; cy < 16; cy++) {
    for (size_t cx = 0; cx < 8; cx++) {
      if (cursor) {
        draw_pixel(x * 9 + cx, y * 16 + cy, 0xbebebe, fb);
      } else {
        draw_pixel(x * 9 + cx, y * 16 + cy, 0, fb);
      }
    }
  }
  if (fb == selected_framebuffer) {
    line_updated[y] = true;
  }
  cursor_drawn[fb] = cursor;
}
static void draw_character(size_t x, size_t y, unsigned char c, size_t fb) {
  if (x >= width || y >= height) {
    return;
  }
  if (c >= 128) {
    if (cursor_drawn[fb]) {
      draw_cursor(x, y, fb, 0);
    }
    return;
  }
  for (size_t cy = 0; cy < 16; cy++) {
    for (size_t cx = 0; cx < 8; cx++) {
      if (font[c * 16 + cy] & 128 >> cx) {
        draw_pixel(x * 9 + cx, y * 16 + cy, 0xbebebe, fb);
      } else {
        draw_pixel(x * 9 + cx, y * 16 + cy, 0, fb);
      }
    }
  }
  if (fb == selected_framebuffer) {
    line_updated[y] = true;
  }
}
void put_character(char c, size_t fb, bool defer_cursor) {
  switch (c) {
  case '\n':
    if (cursor_drawn[fb]) {
      draw_cursor(current_x[fb], current_y[fb], fb, 0);
    }
    current_x[fb] = 0;
    current_y[fb]++;
    break;
  case '\t':;
    for (size_t i = 0; i < 8; i++) {
      put_character(' ', fb, defer_cursor);
    }
    break;
  default:
    draw_character(current_x[fb], current_y[fb], c, fb);
    current_x[fb]++;
  }
  if (current_x[fb] == width) {
    current_x[fb] = 0;
    current_y[fb]++;
  }
  if (current_y[fb] == height) {
    memmove(framebuffer[fb], framebuffer[fb] + 16 * fb_pitch, (fb_height - 16) * fb_pitch);
    memset(framebuffer[fb] + (fb_height - 16) * fb_pitch, 0, 16 * fb_pitch);
    if (fb == selected_framebuffer) {
      screen_updated = true;
    }
    current_y[fb]--;
  }
  if (!defer_cursor) {
    draw_cursor(current_x[fb], current_y[fb], fb, 1);
  }
}
void fb_backspace(size_t fb) {
  if (current_x[fb]) {
    draw_cursor(current_x[fb], current_y[fb], fb, 0);
    current_x[fb]--;
    draw_character(current_x[fb], current_y[fb], ' ', fb);
    draw_cursor(current_x[fb], current_y[fb], fb, 1);
  }
}
void switch_framebuffer(size_t fb) {
  selected_framebuffer = fb;
  send_ipc_call("fbd", IPC_FBD_COPY, 0, 0, 0, (uintptr_t) framebuffer[fb], fb_height * fb_pitch);
}
void update_fb(void) {
  if (screen_updated) {
    send_ipc_call("fbd", IPC_FBD_COPY, 0, 0, 0, (uintptr_t) framebuffer[selected_framebuffer], fb_height * fb_pitch);
    memset(line_updated, 0, height);
    screen_updated = false;
    return;
  }
  for (size_t i = 0; i < height; i++) {
    if (line_updated[i]) {
      send_ipc_call("fbd", IPC_FBD_COPY, i * 16 * fb_pitch, 0, 0, (uintptr_t) framebuffer[selected_framebuffer] + i * 16 * fb_pitch, 16 * fb_pitch);
      line_updated[i] = false;
    }
  }
}

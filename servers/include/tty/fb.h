#pragma once
#include <stdbool.h>
#include <stddef.h>

void fb_backspace(size_t);
void put_character(char, size_t, bool);
void setup_fb(void);
void switch_framebuffer(size_t);
void update_fb(void);

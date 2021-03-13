#pragma once
#include <stdbool.h>
#include <stddef.h>

extern bool call_blocked;
extern size_t selected_framebuffer;

char get_character(void);
void setup_kbd(void);

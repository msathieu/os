#pragma once
#include <stdio.h>

typedef void (*builtin_t)(char*, FILE*);

void builtin_exit(char*, FILE*);
void builtin_export(char*, FILE*);

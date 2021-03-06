#pragma once
#include <__/freestanding/stdio.h>
#include <sys/types.h>

struct FILE {
  size_t fd;
};
extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int fclose(FILE* file);
int fgetc(FILE* file);
int fileno(FILE* file);
FILE* fopen(const char* restrict path, const char* restrict mode);
int fputc(int c, FILE* file);
int fputs(const char* restrict str, FILE* restrict file);
size_t fread(void* restrict buffer, size_t size, size_t num, FILE* restrict file);
int fseek(FILE* file, long position, int mode);
int fseeko(FILE* file, off_t position, int mode);
long ftell(FILE* file);
off_t ftello(FILE* file);
size_t fwrite(const void* restrict buffer, size_t size, size_t num, FILE* restrict file);
int puts(const char* str);
int vfprintf(FILE* restrict file, const char* restrict format, va_list args);

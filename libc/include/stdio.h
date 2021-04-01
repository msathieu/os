#pragma once
#include <__/freestanding/stdio.h>
#define EOF -1
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct {
  size_t fd;
} FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int fclose(FILE*);
int fgetc(FILE*);
int fileno(FILE*);
FILE* fopen(const char* restrict, const char* restrict);
int fprintf(FILE* restrict, const char* restrict, ...);
int fputc(int, FILE*);
int fputs(const char* restrict, FILE* restrict);
size_t fread(void* restrict, size_t, size_t, FILE* restrict);
int fseek(FILE*, long, int);
size_t fwrite(const void* restrict, size_t, size_t, FILE* restrict);
int getc(FILE*);
int putc(int, FILE*);
int puts(const char*);
int vfprintf(FILE* restrict, const char* restrict, va_list);

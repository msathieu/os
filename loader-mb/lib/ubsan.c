#include <stdint.h>
#include <stdio.h>

struct source_location {
  const char* file;
  uint32_t line;
};

void __ubsan_handle_type_mismatch_v1(struct source_location* source) {
  printf("Undefined behavior: %s:%d\n", source->file, source->line);
}
void __ubsan_handle_pointer_overflow(struct source_location* source) {
  printf("Undefined behavior: %s:%d\n", source->file, source->line);
}
void __ubsan_handle_sub_overflow(struct source_location* source) {
  printf("Undefined behavior: %s:%d\n", source->file, source->line);
}
void __ubsan_handle_add_overflow(struct source_location* source) {
  printf("Undefined behavior: %s:%d\n", source->file, source->line);
}
void __ubsan_handle_out_of_bounds(struct source_location* source) {
  printf("Undefined behavior: %s:%d\n", source->file, source->line);
}
void __ubsan_handle_mul_overflow(struct source_location* source) {
  printf("Undefined behavior: %s:%d\n", source->file, source->line);
}
void __ubsan_handle_float_cast_overflow(struct source_location* source) {
  printf("Undefined behavior: %s:%d\n", source->file, source->line);
}
void __ubsan_handle_negate_overflow(struct source_location* source) {
  printf("Undefined behavior: %s:%d\n", source->file, source->line);
}
void __ubsan_handle_shift_out_of_bounds(struct source_location* source) {
  printf("Undefined behavior: %s:%d\n", source->file, source->line);
}
void __ubsan_handle_builtin_unreachable(struct source_location* source) {
  printf("Undefined behavior: %s:%d\n", source->file, source->line);
}

#pragma once
#define panic(msg) _panic(msg, __FILE__, __LINE__, __FUNCTION__)

_Noreturn void _panic(const char*, const char*, int, const char*);

#pragma once
#include <sys/types.h>
#define CLOCKS_PER_SEC 1000

struct timespec {
  time_t tv_sec;
  long tv_nsec;
};
struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
};
typedef long clock_t;

time_t time(time_t* ptr);

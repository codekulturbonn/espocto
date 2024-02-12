#include "console.h"

#ifdef TARGET_NATIVE

void
console_printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  fflush(stdout);
}

#else

void
console_printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buf[256];
  vsnprintf(buf, sizeof(buf), fmt, args);
  Serial.print(buf);
  va_end(args);
}

#endif
#ifdef TARGET_NATIVE
# include <unistd.h>
# include <stdarg.h>
# include <stdio.h>

#else
# include <Arduino.h>
#endif

void console_printf(const char* fmt, ...);

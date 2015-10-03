#include <stdio.h>
#include <stdarg.h>
#include "rzdebug.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#define RZ_COUNT_OF(x) (sizeof(x) / sizeof(x[0]))

int g_assert_on_failure = 1;
int g_log_on_failure = 1;

void __cdecl internal_dbg_out(const char* format, ...)
{
    va_list args;
    char message[1024];

    va_start(args, format);
    vsprintf_s(message, RZ_COUNT_OF(message), format, args);
    va_end(args);

    printf(message);
#ifdef _WIN32
    OutputDebugStringA(message);
#endif
}

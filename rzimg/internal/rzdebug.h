#ifndef _RZDEBUG_H_
#define _RZDEBUG_H_

#include <assert.h>

extern int g_assert_on_failure;
extern int g_log_on_failure;

#ifdef _DEBUG

void __cdecl internal_dbg_out(const char* format, ...);

#define DBG(format, ...) { internal_dbg_out(format, __VA_ARGS__); }

#else // _DEBUG

#define DBG(format, ...) {}

#endif

// error handling

#define CHECK_ERROR(success_cond, res, format, ...) \
    { \
        if (!success_cond) \
        { \
            if (g_assert_on_failure) { assert(0); } \
            if (g_log_on_failure) { DBG(format, __VA_ARGS__); } \
            result = res; \
            goto cleanup; \
        } \
    } \

#endif // _RZDEBUG_H_

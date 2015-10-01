#ifndef _RZIMG_H_
#define _RZIMG_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//=============================================================================
// common headers
//=============================================================================

#include <stdint.h>

//=============================================================================
// types
//=============================================================================

typedef enum
{
    rz_false = 0,
    rz_true
} rz_bool;

typedef enum
{
    rz_success          =  0,
    rz_fail             = -1,
    rz_invalid_param    = -2,
    rz_out_of_memory    = -3,
    rz_file_not_found   = -4,
    rz_access_denied    = -5,
    rz_invalid_file     = -6,
} rz_result;

// Formats are specified in little endian order. For example, rgba means the
// uint32_t layout is 0xaabbggrr
typedef enum
{
    rz_format_unknown = 0,
    rz_format_rgba32,
    rz_format_bgra32,
} rz_format;

//=============================================================================
// bitmap functions
//=============================================================================

rz_result rz_load_bmp(const char* filename, rz_format format,
    uint8_t** pixels, int* width, int* height);


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // _RZIMG_H_
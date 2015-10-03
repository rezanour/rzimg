#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <assert.h>
#include <emmintrin.h>
#include <tmmintrin.h>
#include "rzimg.h"
#include "internal/rzdebug.h"

static const uint16_t BMP_SIGNATURE = 0x4D42;

#pragma pack(push, 1)

typedef struct
{
    uint16_t signature;
    uint32_t file_size;
    uint32_t unused;
    uint32_t data_offset;
} bmp_header_t;

typedef struct
{
    uint32_t header_size;
    int32_t  width;
    int32_t  height;
    uint16_t num_planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t data_size;
    int32_t  hres;
    int32_t  vres;
    uint32_t num_pal_entries;
    uint32_t num_important_colors;
} dib_header_t;

#pragma pack(pop)

rz_result rz_load_bmp(const char* filename, rz_format format,
    uint8_t** pixels, int* width, int* height)
{
    errno_t err = 0;
    FILE* file = NULL;
    uint8_t* data = NULL;
    rz_result result = rz_success;
    bmp_header_t bmp_header;
    dib_header_t* dib_header = NULL;
    uint8_t* src = NULL;
    uint32_t* dst = NULL;
    int i;
    size_t items_read;

    // validate output pointers
    CHECK_ERROR(
        (pixels && width && height),
        rz_invalid_param, "Output pointers invalid.\n");

    // init out params
    *pixels = NULL;
    *width = *height = 0;

    // currently only support rgba & bgra
    CHECK_ERROR(
        (format == rz_format_rgba32 || format == rz_format_bgra32),
        rz_invalid_param, "Specified format is not supported.\n");

    // open the file and try reading base bmp header
    err = fopen_s(&file, filename, "rb");
    CHECK_ERROR(err == 0, (err == ENOENT ? rz_file_not_found : rz_fail),
        "Error opening file.\n");

    items_read = fread(&bmp_header, sizeof(bmp_header_t), 1, file);
    CHECK_ERROR(items_read == 1, rz_invalid_file, "Failed reading bmp header.\n");

    // verify it's a bmp
    CHECK_ERROR(bmp_header.signature == BMP_SIGNATURE, rz_invalid_file,
        "File doens't appear to be bmp.\n");

    // allocate buffer to hold file contents
    data = (uint8_t*)malloc(bmp_header.file_size);
    CHECK_ERROR(data != 0, rz_out_of_memory, "Failed allocating buffer.\n");

    // read in the rest of the file into memory
    items_read = fread(data, bmp_header.file_size - sizeof(bmp_header_t), 1, file);
    CHECK_ERROR(items_read == 1, rz_invalid_file, "File appears truncated.\n");

    // get dib header (right after the bmp header)
    dib_header = (dib_header_t*)data;

    // ensure it's dib (header size should be 40)
    CHECK_ERROR(dib_header->header_size == 40, rz_invalid_file, "Invalid dib header.\n");

    // currently only support uncompressed 24bpp BMP
    CHECK_ERROR((dib_header->compression == 0 && dib_header->bpp == 24),
        rz_fail, "Only uncompressed 24bpp bmp supported currently.\n");

    // allocate pixel buffer
    *pixels = (uint8_t*)malloc(dib_header->width * dib_header->height * sizeof(uint32_t));
    CHECK_ERROR((*pixels), rz_out_of_memory, "Failed allocating pixel buffer.\n");

    *width = dib_header->width;
    *height = dib_header->height;

    // Get start of pixel data
    src = (uint8_t*)(data + bmp_header.data_offset - sizeof(bmp_header_t));
    dst = (uint32_t*)(*pixels);

    // prep masks
    __m128i alpha = _mm_set_epi8(0xFF, 0, 0, 0, 0xFF, 0, 0, 0, 0xFF, 0, 0, 0, 0xFF, 0, 0, 0);
    __m128i mask;
    switch (format)
    {
    case rz_format_bgra32:
        mask = _mm_set_epi8(0x80, 11, 10, 9, 0x80, 8, 7, 6, 0x80, 5, 4, 3, 0x80, 2, 1, 0);
        break;

    default:
        assert(0);
        __fallthrough;
    case rz_format_rgba32:
        mask = _mm_set_epi8(0x80, 9, 10, 11, 0x80, 6, 7, 8, 0x80, 3, 4, 5, 0x80, 0, 1, 2);
        break;
    }

    // stream 4 pixels at a time
    for (i = 0; i < dib_header->width * dib_header->height; i += 4)
    {
        __m128i block = _mm_loadu_si128((const __m128i*)&src[i * 3]);
        __m128i expanded = _mm_shuffle_epi8(block, mask);
        _mm_storeu_si128((__m128i*)&dst[i], _mm_or_si128(expanded, alpha));
    }

cleanup:
    if (data)
    {
        free(data);
    }
    if (file)
    {
        fclose(file);
    }
    if ((*pixels) && result != rz_success)
    {
        free(*pixels);
        *pixels = NULL;
        *width = *height = 0;
    }

    return result;
}


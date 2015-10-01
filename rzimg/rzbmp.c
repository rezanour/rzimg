#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <assert.h>
#include "rzimg.h"

#pragma pack(push, 1)

typedef struct
{
    uint16_t signature;    // 0x4D42 for BMP
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

    // if pixels or pixels_size is null, return error
    if (!pixels || !width || !height)
    {
        result = rz_invalid_param;
        goto cleanup;
    }

    // init out params
    *pixels = NULL;
    *width = *height = 0;

    // currently only support rgba & bgra
    if (format != rz_format_rgba32 && format != rz_format_bgra32)
    {
        result = rz_invalid_param;
        goto cleanup;
    }

    // open the file and try reading base bmp header
    err = fopen_s(&file, filename, "rb");
    if (err)
    {
        result = (err == ENOENT) ? rz_file_not_found : rz_fail;
        goto cleanup;
    }

    if (fread(&bmp_header, sizeof(bmp_header_t), 1, file) != 1)
    {
        result = rz_invalid_file;
        goto cleanup;
    }

    // verify it's a bmp
    if (bmp_header.signature != 0x4D42)
    {
        result = rz_invalid_file;
        goto cleanup;
    }

    // allocate buffer to hold file contents
    data = (uint8_t*)malloc(bmp_header.file_size);
    if (!data)
    {
        result = rz_out_of_memory;
        goto cleanup;
    }

    // read in the rest of the file into memory
    if (fread(data, bmp_header.file_size - sizeof(bmp_header_t), 1, file) != 1)
    {
        result = rz_invalid_file;
        goto cleanup;
    }

    // get dib header (right after the bmp header)
    dib_header = (dib_header_t*)data;

    // ensure it's dib (header size should be 40)
    if (dib_header->header_size != 40)
    {
        result = rz_invalid_file;
        goto cleanup;
    }

    // currently only support uncompressed 24bpp BMP
    if (dib_header->compression != 0 || dib_header->bpp != 24)
    {
        result = rz_fail;
        goto cleanup;
    }

    // allocate pixel buffer
    *pixels = (uint8_t*)malloc(dib_header->width * dib_header->height * sizeof(uint32_t));
    if (!(*pixels))
    {
        result = rz_out_of_memory;
        goto cleanup;
    }

    *width = dib_header->width;
    *height = dib_header->height;

    // Get start of pixel data
    src = (uint8_t*)(data + bmp_header.data_offset - sizeof(bmp_header_t));
    dst = (uint32_t*)(*pixels);

    // read pixels
    for (i = 0; i < dib_header->width * dib_header->height * 3; i += 3) // * 3 for size since 24bpp
    {
        switch (format)
        {
        case rz_format_bgra32:
            *dst = 0xFF000000 |
                ((uint32_t)src[i + 2] << 16) |
                ((uint32_t)src[i + 1] << 8) |
                (uint32_t)src[i];
            break;

        case rz_format_rgba32:
            *dst = 0xFF000000 |
                ((uint32_t)src[i] << 16) |
                ((uint32_t)src[i + 1] << 8) |
                (uint32_t)src[i + 2];
            break;
        }
        ++dst;
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


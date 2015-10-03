#include <stdio.h>
#include <malloc.h>
#include <rzimg.h>

#ifdef _WIN32
#include <Windows.h>
void show_image(int width, int height, rz_format format, const uint8_t* pixels)
{
    HWND hwnd = NULL;

    hwnd = CreateWindow(WC_DIALOG, L"rzimg", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
        width, height, NULL, NULL, GetModuleHandle(NULL), NULL);

    if (hwnd)
    {
        HDC hdc, hmemdc;
        HBITMAP hbmp = NULL;

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        hdc = GetDC(hwnd);
        hmemdc = CreateCompatibleDC(hdc);

        switch (format)
        {
        case rz_format_bgra32:
            hbmp = CreateBitmap(width, height, 1, 32, pixels);
            break;

        case rz_format_rgba32:
            {
                int i;
                uint32_t* swapped = (uint32_t*)malloc(sizeof(uint32_t) * width * height);
                uint32_t* dst = swapped;
                const uint32_t* src = (const uint32_t*)pixels;
                for (i = 0; i < width * height; ++i, ++src, ++dst)
                {
                    *dst = 0xFF000000 |
                        ((*src & 0x000000FF) << 16) |
                        (*src & 0x0000FF00) |
                        ((*src & 0x00FF0000) >> 16);
                }
                hbmp = CreateBitmap(width, height, 1, 32, swapped);
                free(swapped);
            }
            break;

        default:
            printf("invalid!\n");
            break;
        }

        if (hbmp)
        {
            SelectObject(hmemdc, hbmp);

            StretchBlt(hdc, 0, height, width, -height, hmemdc, 0, 0, width, height, SRCCOPY);
            DeleteObject(hbmp);
        }

        DeleteDC(hmemdc);

        ReleaseDC(hwnd, hdc);

        Sleep(2000);

        DestroyWindow(hwnd);
    }
}
#else
void show_image(int width, int height, rz_format format, const uint8_t* pixels);
#endif

int main(int argc, char* args[])
{
    rz_result result = rz_success;
    uint8_t* pixels = NULL;
    int width, height;

    // unused
    (void)argc;
    (void)args;

    // test 24bpp bmp
    result = rz_load_bmp("test.bmp", rz_format_rgba32, &pixels, &width, &height);
    if (result != rz_success)
    {
        printf("Failed to load 24bpp bmp. %d\n", result);
        goto cleanup;
    }

    show_image(width, height, rz_format_rgba32, pixels);

    printf("Success!\n");

cleanup:
    if (pixels)
        free(pixels);

    return result;
}
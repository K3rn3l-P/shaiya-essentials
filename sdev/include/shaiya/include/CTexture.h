#pragma once
#include <shaiya/include/common.h>
#include "include/shaiya/common.h"

namespace shaiya
{
#pragma pack(push, 1)
    struct STexture
    {
        PAD(4);                      // 0x00
        char fileName[256];          // 0x04
        char path[256];              // 0x104
        LPDIRECT3DTEXTURE9 texture;  // 0x204
        // 0x208
    };
#pragma pack(pop)

    static_assert(sizeof(STexture) == 0x208);

#pragma pack(push, 1)
    struct CTexture
    {
        LPDIRECT3DTEXTURE9 texture;
        PAD(4);
        D2D_SIZE_F size;
        // 0x10

        // Variante originale (int)
        static int CreateFromFile(CTexture* texture, const char* path, const char* fileName, int w, int h);
        static void Render(CTexture* texture, int x, int y, float z);
        static void Render(CTexture* texture, D3DCOLOR diffuse,
            int x, int y, float z,
            int w, int h,
            float a, float b, float c, float d
        );

        // Variante nuova (long) - compatibile con hook texture
        static void CreateFromFile(CTexture* texture, const char* path, const char* fileName, long w, long h);
        static void Render(CTexture* texture, long x, long y, float z);
    };
#pragma pack(pop)

    static_assert(sizeof(CTexture) == 0x10);
}

#include "include/shaiya/include/CTexture.h"
using namespace shaiya;

int CTexture::CreateFromFile(CTexture* texture, const char* path, const char* fileName, int w, int h)
{
    typedef int(__thiscall* LPFN)(CTexture*, const char*, const char*, int, int);
    return (*(LPFN)0x57B560)(texture, path, fileName, w, h);
}

// NUOVE FUNZIONI:
void CTexture::Render(CTexture* texture, int x, int y, float z)
{
    typedef void(__thiscall* LPFN)(CTexture*, int, int, float);
    (*(LPFN)0x57B680)(texture, x, y, z);
}
void CTexture::Render(CTexture* texture, long x, long y, float z)
{
    typedef void(__thiscall* LPFN)(CTexture*, long, long, float);
    (*(LPFN)0x57B680)(texture, x, y, z);
}
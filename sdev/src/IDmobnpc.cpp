#include "include/shaiya/include/IDmobnpc.h"
#include <windows.h>

void EnableCheat() {
    // Allocare memoria per il nuovo codice
    void* newmem = VirtualAlloc(0, 2048, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!newmem) return;

    // Codice assembly tradotto in C++
    unsigned char* code = (unsigned char*)newmem;
    code[0] = 0xA0; // mov al, [0090D1D4]
    code[1] = 0xD4;
    code[2] = 0xD1;
    code[3] = 0x90;
    code[4] = 0x00;
    code[5] = 0x3C; // cmp al, 01
    code[6] = 0x01;
    code[7] = 0x74; // je originalcode
    code[8] = 0x02;
    code[9] = 0x3C; // cmp al, 02
    code[10] = 0x02;
    code[11] = 0x74; // je originalcode
    code[12] = 0x02;
    code[13] = 0x3C; // cmp al, 03
    code[14] = 0x03;
    code[15] = 0x0F; // sete al
    code[16] = 0x94;
    code[17] = 0xC0;
    code[18] = 0xC3; // ret

    // Codice originale
    unsigned char* originalcode = code + 19;
    originalcode[0] = 0xB0; // mov al, 01
    originalcode[1] = 0x01;
    originalcode[2] = 0xC3; // ret
    originalcode[3] = 0xE9; // jmp returnhere
    originalcode[4] = 0x00;
    originalcode[5] = 0x00;
    originalcode[6] = 0x00;
    originalcode[7] = 0x00;

    // Indirizzo di ritorno
    unsigned char* returnhere = originalcode + 8;

    // Inserire il salto al nuovo codice
    DWORD oldProtect;
    VirtualProtect((void*)0x004E5876, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    *(unsigned char*)0x004E5876 = 0xE9; // jmp newmem
    *(DWORD*)(0x004E5876 + 1) = (DWORD)newmem - 0x004E5876 - 5;
    VirtualProtect((void*)0x004E5876, 5, oldProtect, &oldProtect);
}
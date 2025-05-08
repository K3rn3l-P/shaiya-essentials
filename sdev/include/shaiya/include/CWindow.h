#pragma once
#include <shaiya/include/common.h>
#include "include/shaiya/common.h"

namespace shaiya
{
    #pragma pack(push, 1)
    struct CWindow
    {
        void* vftable;                   //0x00
        D2D_POINT_2U pos;                //0x04
        D2D_SIZE_U size;                 //0x0C
        bool32_t leftMouseButtonDown;    //0x14
        D2D_POINT_2U leftMouseClickPos;  //0x18
        bool32_t visible;                //0x20
        // 0x24

        static void Draw(LPDIRECT3DBASETEXTURE9 texture, int x, int y);

        // NUOVE FUNZIONI
        // Dichiarazione della funzione statica
        static void UpdateWindowTitle();
        // Aggiungi le dichiarazioni delle nuove funzioni
        static int get_ping_and_packet_loss(const char* server_address, int port);
        static int calculate_packet_loss(int num_attempts, int packet_loss);
    };
    #pragma pack(pop) 

    static_assert(sizeof(CWindow) == 0x24);
}

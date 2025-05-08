#include <windows.h>
#include <util/util.h>
#include "include/static.h"

// Inclusioni Shaiya core
#include "include/shaiya/include/CPlayerData.h"
#include "shaiya/include/common/Country.h"
#include "shaiya/include/common/NpcTypes.h"
#include "include/shaiya/include/CNetwork.h"
#include "include/shaiya/include/buff_transparency.h"

using namespace shaiya;

namespace hook
{
    DWORD return_address = 0x004D744C;
    DWORD original_function = 0x04B57B0;
    bool is_transparent = false;
    DWORD last_toggle_time = 0;
    const DWORD toggle_delay = 300;

    enum HelpMenuButtonIndex {
        BasicPlay, PlayGuide, Interface, Blessing, BasicAction, PlayMode
    };

#pragma pack(push, 1)
    struct VetRewardLevelOutgoing {
        UINT16 opcode{ 0x218 };
    };
#pragma pack(pop)

    void set_help_menu_npc(HelpMenuButtonIndex index)
    {
        // Rimozione controllo finestra attiva
        switch (index)
        {
        case BasicPlay:
            g_pPlayerData->npcType = std::to_underlying(NpcType::Blacksmith);
            g_pPlayerData->npcTypeId = 40;
            g_pPlayerData->windowType = WindowType::Blacksmith;
            break;
        case PlayGuide:
            g_pPlayerData->npcType = std::to_underlying(NpcType::Merchant);
            g_pPlayerData->npcTypeId = 248;
            g_pPlayerData->windowType = WindowType::Recreation;
            break;
        case Interface:
        {
            VetRewardLevelOutgoing outgoing{};
            CNetwork::Send(&outgoing, sizeof(outgoing));
            g_var->killLv = 0;
            g_var->deathLv = 0;
            g_pPlayerData->npcType = std::to_underlying(NpcType::VetManager);
            g_pPlayerData->npcTypeId = (g_pPlayerData->country == Country::Light ? 1 : 2);
            break;
        }
        case Blessing:
            g_pPlayerData->npcType = std::to_underlying(NpcType::Merchant);
            g_pPlayerData->npcTypeId = (g_pPlayerData->country == Country::Light ? 179 : 180);
            g_pPlayerData->windowType = WindowType::BankTeller;
            break;
        case BasicAction:
            g_pPlayerData->npcType = std::to_underlying(NpcType::GuildMaster);
            g_pPlayerData->npcTypeId = (g_pPlayerData->country == Country::Light ? 1 : 2);
            g_pPlayerData->windowType = WindowType::GuildMaster;
            break;
        case PlayMode:
            g_pPlayerData->npcType = std::to_underlying(NpcType::Merchant);
            g_pPlayerData->npcTypeId = 437;
            g_pPlayerData->windowType = WindowType::Merchant;
            break;
        }
        g_pPlayerData->npcIcon = 55;
        g_pPlayerData->textBuffer[0] = '\0';
    }

    void __declspec(naked) custom_buff_color()
    {
        __asm {
            pushad

            // CTRL + 0 toggle
            push 0x30
            call GetAsyncKeyState
            shr ax, 0xF
            cmp ax, 1
            jne check_keys
            push 0x11
            call GetAsyncKeyState
            shr ax, 0xF
            cmp ax, 1
            jne check_keys

            call GetTickCount
            mov ecx, last_toggle_time
            sub eax, ecx
            cmp eax, toggle_delay
            jl check_keys

            call GetTickCount
            mov last_toggle_time, eax

            movzx eax, byte ptr[is_transparent]
                xor al, 1
                    mov byte ptr[is_transparent], al

                    check_keys :
                // CTRL + 1..6 gestione NPC
                push 0x11
                    call GetAsyncKeyState
                    shr ax, 0xF
                    cmp ax, 1
                    jne skip_keys

                    call GetTickCount
                    mov ecx, last_toggle_time
                    sub eax, ecx
                    cmp eax, toggle_delay
                    jl skip_keys

                    call GetTickCount
                    mov last_toggle_time, eax

                    push 0x31
                    call GetAsyncKeyState
                    shr ax, 0xF
                    cmp ax, 1
                    jne k2
                    push PlayMode
                    call set_help_menu_npc
                    add esp, 4
                    jmp skip_keys

                    k2 :
                push 0x32
                    call GetAsyncKeyState
                    shr ax, 0xF
                    cmp ax, 1
                    jne k3
                    push PlayGuide
                    call set_help_menu_npc
                    add esp, 4
                    jmp skip_keys

                    k3 :
                push 0x33
                    call GetAsyncKeyState
                    shr ax, 0xF
                    cmp ax, 1
                    jne k4
                    push BasicPlay
                    call set_help_menu_npc
                    add esp, 4
                    jmp skip_keys

                    k4 :
                push 0x34
                    call GetAsyncKeyState
                    shr ax, 0xF
                    cmp ax, 1
                    jne k5
                    push Blessing
                    call set_help_menu_npc
                    add esp, 4
                    jmp skip_keys

                    k5 :
                push 0x35
                    call GetAsyncKeyState
                    shr ax, 0xF
                    cmp ax, 1
                    jne k6
                    push BasicAction
                    call set_help_menu_npc
                    add esp, 4
                    jmp skip_keys

                    k6 :
                push 0x36
                    call GetAsyncKeyState
                    shr ax, 0xF
                    cmp ax, 1
                    jne skip_keys
                    push Interface
                    call set_help_menu_npc
                    add esp, 4

                    skip_keys:
                popad

                    cmp byte ptr[is_transparent], 1
                    jne normal_color
                    push 0x50FFFFFF
                    jmp call_color

                    normal_color :
                push 0xFFFFFFFF

                    call_color :
                    mov ecx, ebp
                    mov eax, original_function
                    call eax
                    jmp return_address
        }
    }

    void buff_transparency()
    {
        DWORD hook_location = 0x004D7447;
        BYTE patch[5] = { 0xE9 };
        DWORD rel_addr = (DWORD)&custom_buff_color - (hook_location + 5);
        memcpy(patch + 1, &rel_addr, 4);
        util::write_memory((void*)hook_location, patch, 5);

        BYTE nops[4] = { 0x90, 0x90, 0x90, 0x90 };
        util::write_memory((void*)(hook_location + 5), nops, 4);
    }
}


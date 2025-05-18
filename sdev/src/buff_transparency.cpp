#include <windows.h>
#include <util/util.h>
#include "include/static.h"

// Core Shaiya
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
    ULONGLONG last_toggle_time = 0;
    const DWORD toggle_delay = 300;

    enum HelpMenuButtonIndex {
        BasicPlay, PlayGuide, Interface, Blessing, BasicAction, PlayMode
    };

#pragma pack(push, 1)
    struct VetRewardLevelOutgoing {
        UINT16 opcode{ 0x218 };
    };
#pragma pack(pop)

    HelpMenuButtonIndex currentOpened = HelpMenuButtonIndex(-1);

    void set_help_menu_npc(HelpMenuButtonIndex index)
    {
        // Chiudi se ri-premuto
        if (g_pPlayerData->windowType != WindowType::None && currentOpened == index)
        {
            g_pPlayerData->windowType = WindowType::None;
            currentOpened = HelpMenuButtonIndex(-1);
            return;
        }

        // Blocca apertura sovrapposta
        if (g_pPlayerData->windowType != WindowType::None)
        {
            Static::MsgTextOut(31, 806, 12); // Messaggio: finestra già aperta
            return;
        }

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
        currentOpened = index;
    }

    void handle_key_combo()
    {
        // Controlla prima CTRL+0 per la trasparenza
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(0x30) & 0x8000))
        {
            ULONGLONG now = GetTickCount64();
            if (now - last_toggle_time >= toggle_delay)
            {
                is_transparent = !is_transparent;
                last_toggle_time = now;
            }
            return;  // Esci dopo aver gestito CTRL+0
        }

        // Poi gestisci gli altri shortcut CTRL+1-6
        if (!(GetAsyncKeyState(VK_CONTROL) & 0x8000))
            return;

        ULONGLONG now = GetTickCount64();
        if (now - last_toggle_time < toggle_delay)
            return;

        HelpMenuButtonIndex map[6] = {
            PlayMode,     // CTRL + 1
            PlayGuide,    // CTRL + 2
            BasicPlay,    // CTRL + 3
            Blessing,     // CTRL + 4
            BasicAction,  // CTRL + 5
            Interface     // CTRL + 6
        };

        for (int i = 0; i < 6; ++i)
        {
            if (GetAsyncKeyState(0x31 + i) & 0x8000)
            {
                set_help_menu_npc(map[i]);
                last_toggle_time = now;
                return;
            }
        }
    }

    __declspec(naked) void custom_buff_color()
    {
        __asm {
            pushad
        }

        // Sposta la gestione di CTRL+0 in handle_key_combo()
        handle_key_combo();

        __asm {
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

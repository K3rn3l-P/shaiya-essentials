#include <map>
#include <util/util.h>
#include "include/main.h"
#include "include/static.h"
#include "include/shaiya/include/CCharacter.h"
#include "include/shaiya/include/CDataFile.h"
#include "include/shaiya/include/HexColor.h"
#include "include/shaiya/include/ItemInfo.h"

#include <chrono>  // Per il controllo del tempo
#include <cmath>   // Per fmodf, fabsf

using namespace shaiya;

namespace name_color
{
    const std::map<uint16_t, HexColor> g_itemRangeToColor
    {
        { 1,  HexColor::LightBlue }, { 2,  HexColor::Blue   }, { 3,  HexColor::Green  },
        { 4,  HexColor::Yellow    }, { 5,  HexColor::Orange }, { 6,  HexColor::Red    },
        { 7,  HexColor::Pink      }, { 8,  HexColor::Purple }, { 9,  HexColor::Gray   },
        {10,  HexColor::Black     }, {11,  HexColor::Cyan   }, {12,  HexColor::Magenta},
        {13,  HexColor::Brown     }, {14,  HexColor::Lime   }, {15,  HexColor::Olive  },
        {16,  HexColor::Maroon    }, {17,  HexColor::Navy   }, {18,  HexColor::Teal   },
        {19,  HexColor::Silver    }, {20,  HexColor::Gold   }, {21,  HexColor::Crimson},
        {22,  HexColor::Khaki     }, {23,  HexColor::Lavender}, {24,  HexColor::Peach  },
        {25,  HexColor::Coral     }, {26,  HexColor::Salmon }, {27,  HexColor::Mint   },
        {28,  HexColor::Beige     }, {29,  HexColor::Plum   }, {30,  HexColor::Orchid },
        {31,  HexColor::Rose      }, {32,  HexColor::Wheat  }, {33,  HexColor::Azure  },
        {34,  HexColor::Ivory     }, {35,  HexColor::Snow   }, {36,  HexColor::Honeydew},
        {37,  HexColor::LimeGreen }, {38,  HexColor::LightCoral}, {39, HexColor::LightPink},
        {40,  HexColor::SeaGreen  }, {41,  HexColor::SkyBlue}, {42,  HexColor::SlateGray},
        {43,  HexColor::Turquoise }, {44,  HexColor::VioletRed}, {45, HexColor::SpringGreen},
        {46,  HexColor::Chartreuse}, {47,  HexColor::Sienna }, {48, HexColor::SlateBlue},
        {49,  HexColor::SteelBlue}, {50,  HexColor::Tomato }, {51, HexColor::DarkRed},
        {52,  HexColor::DarkOrange}, {53,  HexColor::DarkViolet}, {54, HexColor::LightYellow},
        {55,  HexColor::LightCyan }, {56,  HexColor::PapayaWhip}, {57, HexColor::Moccasin},
        {58,  HexColor::NavajoWhite}, {59,  HexColor::LemonChiffon}, {60, HexColor::MistyRose}
    };

    // Struttura RGB di supporto
    struct RGB { int r, g, b; };

    // Conversione HSV -> RGB\    
    static RGB HSVtoRGB(float H, float S, float V)
    {
        float C = V * S;
        float Hprime = fmodf(H / 60.0f, 6.0f);
        float X = C * (1.0f - fabsf(fmodf(Hprime, 2.0f) - 1.0f));
        float m = V - C;

        float r1, g1, b1;
        if (0.0f <= Hprime && Hprime < 1.0f) { r1 = C; g1 = X; b1 = 0; }
        else if (Hprime < 2.0f) { r1 = X; g1 = C; b1 = 0; }
        else if (Hprime < 3.0f) { r1 = 0; g1 = C; b1 = X; }
        else if (Hprime < 4.0f) { r1 = 0; g1 = X; b1 = C; }
        else if (Hprime < 5.0f) { r1 = X; g1 = 0; b1 = C; }
        else { r1 = C; g1 = 0; b1 = X; }

        return RGB{
            static_cast<int>((r1 + m) * 255),
            static_cast<int>((g1 + m) * 255),
            static_cast<int>((b1 + m) * 255)
        };
    }

    // Ritorna un colore arcobaleno ciclico senza glitch
    HexColor get_multicolor()
    {
        constexpr float cycleDurationMs = 7000.0f; // 7 secondi per ciclo completo
        static const auto startTime = std::chrono::steady_clock::now();

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

        // Calcolo dell'hue in [0,360)
        float hue = fmodf((elapsed / cycleDurationMs) * 360.0f, 360.0f);
        constexpr float saturation = 1.0f;
        constexpr float value = 1.0f;

        RGB rgb = HSVtoRGB(hue, saturation, value);
        return HexColor(
            0xFF000000 |
            (static_cast<uint32_t>(rgb.r) << 16) |
            (static_cast<uint32_t>(rgb.g) << 8) |
            static_cast<uint32_t>(rgb.b)
        );
    }

    // Funzione esistente per i mob
    HexColor get_mob_name_color(int mobLevel)
    {
        int gap = mobLevel - g_pPlayerData->level;
        if (gap >= 10)
            return HexColor::Gray;
        switch (gap)
        {
        case 9: case 8:   return HexColor::Pink;
        case 7: case 6:   return HexColor::Red;
        case 5: case 4:   return HexColor::Orange;
        case 3: case 2:   return HexColor::Yellow;
        case 1: case 0:
        case -1:          return HexColor::Green;
        case -2: case -3: return HexColor::Blue;
        case -4: case -5: return HexColor::LightBlue;
        default:           return HexColor::White;
        }
    }

    // Ritorna il colore del nome in base all'elmo (range 1 = arcobaleno)
    D3DCOLOR get_helmet_name_color(CCharacter* user)
    {
        auto itemInfo = CDataFile::GetItemInfo(user->helmetType, user->helmetTypeId);
        if (!itemInfo || itemInfo->range == 0)
            return std::to_underlying(HexColor::White);

        auto it = g_itemRangeToColor.find(itemInfo->range);
        if (it != g_itemRangeToColor.end())
        {
            return (itemInfo->range == 1)
                ? std::to_underlying(get_multicolor())
                : std::to_underlying(it->second);
        }
        return std::to_underlying(HexColor::White);
    }
}

// Hook assembly
extern "C" void __declspec(naked) naked_0x4E50D0()
{
    __asm
    {
        push ebx
        push edi
        push esi

        movzx eax, word ptr[esp + 0x10]
        push eax
        call name_color::get_mob_name_color
        add  esp, 4

        pop esi
        pop edi
        pop ebx
        retn 0x4
    }
}

static unsigned u0x453821 = 0x453821;
extern "C" void __declspec(naked) naked_0x45381B()
{
    __asm
    {
        push ebx
        push edi
        push esi

        push esi
        call name_color::get_helmet_name_color
        add  esp, 4
        test eax, eax

        pop esi
        pop edi
        pop ebx
        je original

        mov ebp, eax
        original :
        cmp dword ptr ds : [0x22AA7F8] , ebx
            jmp u0x453821
    }
}

void hook::name_color()
{
    util::detour((void*)0x4E50D0, naked_0x4E50D0, 5);
    util::detour((void*)0x45381B, naked_0x45381B, 6);
}

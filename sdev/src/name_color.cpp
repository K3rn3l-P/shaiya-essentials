#include <map>
#include <util/util.h>
#include "include/main.h"
#include "include/static.h"
#include "include/shaiya/include/CCharacter.h"
#include "include/shaiya/include/CDataFile.h"
#include "include/shaiya/include/HexColor.h"
#include "include/shaiya/include/ItemInfo.h"

// NUOVE FUNZIONI
#include <chrono>
#include <cmath>

using namespace shaiya;

namespace name_color
{
    const std::map<uint16_t, HexColor> g_itemRangeToColor
    {
        { 1, HexColor::LightBlue },
        { 2, HexColor::Blue },
        { 3, HexColor::Green },
        { 4, HexColor::Yellow },
        { 5, HexColor::Orange },
        { 6, HexColor::Red },
        { 7, HexColor::Pink },
        { 8, HexColor::Purple },
        { 9, HexColor::Gray },
        { 10, HexColor::Black },
        { 11, HexColor::Cyan },
        { 12, HexColor::Magenta },
        { 13, HexColor::Brown },
        { 14, HexColor::Lime },
        { 15, HexColor::Olive },
        { 16, HexColor::Maroon },
        { 17, HexColor::Navy },
        { 18, HexColor::Teal },
        { 19, HexColor::Silver },
        { 20, HexColor::Gold },
        { 21, HexColor::Crimson },
        { 22, HexColor::Khaki },
        { 23, HexColor::Lavender },
        { 24, HexColor::Peach },
        { 25, HexColor::Coral },
        { 26, HexColor::Salmon },
        { 27, HexColor::Mint },
        { 28, HexColor::Beige },
        { 29, HexColor::Plum },
        { 30, HexColor::Orchid },
        { 31, HexColor::Rose },
        { 32, HexColor::Wheat },
        { 33, HexColor::Azure },
        { 34, HexColor::Ivory },
        { 35, HexColor::Snow },
        { 36, HexColor::Honeydew },
        { 37, HexColor::LimeGreen },
        { 38, HexColor::LightCoral },
        { 39, HexColor::LightPink },
        { 40, HexColor::SeaGreen },
        { 41, HexColor::SkyBlue },
        { 42, HexColor::SlateGray },
        { 43, HexColor::Turquoise },
        { 44, HexColor::VioletRed },
        { 45, HexColor::SpringGreen },
        { 46, HexColor::Chartreuse },
        { 47, HexColor::Sienna },
        { 48, HexColor::SlateBlue },
        { 49, HexColor::SteelBlue },
        { 50, HexColor::Tomato },
        { 51, HexColor::DarkRed },
        { 52, HexColor::DarkOrange },
        { 53, HexColor::DarkViolet },
        { 54, HexColor::LightYellow },
        { 55, HexColor::LightCyan },
        { 56, HexColor::PapayaWhip },
        { 57, HexColor::Moccasin },
        { 58, HexColor::NavajoWhite },
        { 59, HexColor::LemonChiffon },
        { 60, HexColor::MistyRose }
    };

	// NUOVE FUNZIONI
	// Funzione per convertire HSV in RGB
    struct RGB { int r, g, b; };

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
	// Funzione per ottenere un colore multicolore che cambia nel tempo
    HexColor get_multicolor()
    {
        constexpr float cycleDurationMs = 12000.0f;
        constexpr float saturation = 0.9f;
        constexpr float value = 0.95f;

        auto now = std::chrono::steady_clock::now();
        float ms = static_cast<float>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()
            ).count()
            );

        float hue = fmodf((ms / cycleDurationMs) * 360.0f, 360.0f);

        float C = value * saturation;
        float Hprime = fmodf(hue / 60.0f, 6.0f);
        float X = C * (1.0f - fabsf(fmodf(Hprime, 2.0f) - 1.0f));
        float m = value - C;

        float r = 0, g = 0, b = 0;
        if (0 <= Hprime && Hprime < 1) { r = C; g = X; b = 0; }
        else if (Hprime < 2) { r = X; g = C; b = 0; }
        else if (Hprime < 3) { r = 0; g = C; b = X; }
        else if (Hprime < 4) { r = 0; g = X; b = C; }
        else if (Hprime < 5) { r = X; g = 0; b = C; }
        else { r = C; g = 0; b = X; }

        auto gamma = [](float c, float m) -> uint32_t {
            float corrected = powf(c + m, 0.9f);
            return static_cast<uint32_t>(corrected * 255.0f);
            };

        uint32_t red = gamma(r, m);
        uint32_t green = gamma(g, m);
        uint32_t blue = gamma(b, m);

        return HexColor(0xFF000000 | (red << 16) | (green << 8) | blue);
    }

	// Funzione per ottenere il colore del nome del mob in base al livello
    HexColor get_mob_name_color(int mobLevel)
    {
        int gap = mobLevel - g_pPlayerData->level;
        if (gap >= 10)
            return HexColor::Gray;

        switch (gap)
        {
        case 9: case 8:
            return HexColor::Pink;
        case 7: case 6:
            return HexColor::Red;
        case 5: case 4:
            return HexColor::Orange;
        case 3: case 2:
            return HexColor::Yellow;
        case 1: case 0: case -1:
            return HexColor::Green;
        case -2: case -3:
            return HexColor::Blue;
        case -4: case -5:
            return HexColor::LightBlue;
        default:
            break;
        }

        return HexColor::White;
    }
	// Funzione per ottenere il colore del nome dell'elmo dell'utente
    D3DCOLOR get_helmet_name_color(CCharacter* user)
    {
        auto helmetType = user->equipment.type[EquipmentSlot::Helmet];
        auto helmetTypeId = user->equipment.typeId[EquipmentSlot::Helmet];
        auto itemInfo = CDataFile::GetItemInfo(helmetType, helmetTypeId);

        if (!itemInfo || itemInfo->range == 0)
            return std::to_underlying(HexColor::White);

        auto it = g_itemRangeToColor.find(itemInfo->range);
        if (it != g_itemRangeToColor.end())
        {
            if (itemInfo->range == 1)
                return std::to_underlying(get_multicolor());
            else
                return std::to_underlying(it->second);
        }

        return std::to_underlying(HexColor::White);
    }
}

// Hook colore nome mob
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

// Hook colore nome giocatori/admin
static unsigned u0x453821 = 0x453821;

extern "C" void __declspec(naked) naked_0x45380C()
{
    __asm
    {
        movzx eax, byte ptr[esi + 0x2D4] // isAdmin
        test eax, eax
        je not_admin

        call name_color::get_multicolor
        mov ebp, eax
        jmp done

        not_admin :
        push esi
            call name_color::get_helmet_name_color
            add  esp, 4
            test eax, eax
            je done
            mov ebp, eax

            done :
        cmp dword ptr ds : [0x22AA7F8] , ebx
            jmp u0x453821
    }
}

void hook::name_color()
{
    util::detour((void*)0x4E50D0, naked_0x4E50D0, 5); // mob name color
    util::detour((void*)0x45380C, naked_0x45380C, 6); // admin + user name color
}

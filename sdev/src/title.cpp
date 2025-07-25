// GifAnimator: gestisce una sequenza di CTexture* come animazione tipo GIF.
// Esempio di utilizzo per altre animazioni:
// 1. Crea una nuova istanza: GifAnimator myAnimator(velocitaMs);
// 2. Prepara i frame: std::vector<CTexture*> frames = { tex1, tex2, ... };
// 3. myAnimator.setFrames(frames);
// 4. Per ottenere il frame corrente: myAnimator.getCurrentFrame();
// 5. Usa il frame in CTexture::Render(...)
// Puoi creare più GifAnimator (es: uno per admin, uno per kill, uno per vip, ecc.)
// Esempio per animazione kill:
// GifAnimator killAnimator(80); // 80ms per frame
// std::vector<CTexture*> killFrames = { kill0, kill1, kill2 };
// killAnimator.setFrames(killFrames);
// ...
// selectedTexture = killAnimator.getCurrentFrame();

#include <map>
#include <tuple>
#include <util/util.h>
#include "include/main.h"
#include "include/static.h"
#include "include/shaiya/include/CCharacter.h"
#include "include/shaiya/include/CDataFile.h"
#include "include/shaiya/include/CMonster.h"
#include "include/shaiya/include/CStaticText.h"
#include "include/shaiya/include/ItemInfo.h"
#include "include/shaiya/include/HexColor.h"
#include <set>
#include <include/shaiya/include/CTexture.h>
#include "include/shaiya/include/CPlayerData.h"
#include <vector>
#include <windows.h>
#include "gif_animator.h"

using namespace shaiya;

namespace title
{
    using ItemId = uint32_t;

    constexpr float chat_y_add = 2.50F;

    std::map<ItemId, std::tuple<const char*, HexColor>> items
    {
        { 24028, { "Champion of Teos", HexColor::Red } },
        { 24029, { "Gladiator", HexColor::Green } },
        { 24030, { "Protector", HexColor::Blue } },
        { 24031, { "Archimage", HexColor::Yellow } },
        { 24032, { "Hawk Eye", HexColor::Aqua } },
        { 24033, { "Outlaw", HexColor::Fuchsia } },
        { 24034, { "Healer", HexColor::Maroon } },
        { 24035, { "Hardcore Player", HexColor::DarkGreen } },
        { 24036, { "Duelist", HexColor::NavyBlue } },
        { 24037, { "Commander", HexColor::Olive } },
        { 24038, { "Captain", HexColor::Purple } },
        { 24039, { "Veteran", HexColor::Silver } },
        { 24040, { "War Chief", HexColor::Gray } },
        { 24041, { "Adventurer", HexColor::DarkPurple } },
        { 24042, { "Queen", HexColor::DarkBlueGray } },
        { 24043, { "King", HexColor::Teal } },
        { 24044, { "Baron", HexColor::Maroon } },
        { 24045, { "Baroness", HexColor::DarkGreen } },
        { 24046, { "Mystic", HexColor::NavyBlue } },
        { 24047, { "King of Arena", HexColor::Orange } },
        { 24048, { "Elemental Master", HexColor::Purple } },
        { 24049, { "Witch", HexColor::Teal } },
        { 24050, { "Paladin", HexColor::MediumSpringGreen } },
        { 24051, { "Shaman", HexColor::GoldenRod } },
        { 24052, { "Druid", HexColor::FireBrick } },
        { 24053, { "Death Knight", HexColor::GreenYellow } },
        { 24054, { "High Priestess", HexColor::Chartreuse } },
        { 24055, { "Interloper", HexColor::Crimson } },
        { 24056, { "Crazy Cat Lady", HexColor::HotPink } },
        { 24057, { "Mercenary", HexColor::Salmon } },
        { 24058, { "Salty", HexColor::BlueViolet } },
        { 24059, { "MVP", HexColor::CadetBlue } },
        { 24060, { "Old School", HexColor::PowderBlue } },
        { 24061, { "Cryptic", HexColor::FireBrick } },
        { 24062, { "Chill Player", HexColor::SaddleBrown } },
        { 24063, { "Farmer", HexColor::AntiqueWhite } },
        { 24064, { "Enigmatic", HexColor::LawnGreen } },
        { 24065, { "Rich", HexColor::Gold } },
        { 24066, { "Fairy", HexColor::Orchid } },
        { 24067, { "Survivor", HexColor::SpringGreen } },
        { 24068, { "Untouchable", HexColor::SteelBlue } },
        { 24069, { "Maniac", HexColor::LimeGreen } },
        { 24104, { "Love Fool", HexColor::DeepPink } },
        { 24105, { "Archivist", HexColor::LightSlateBlue } },
        { 24106, { "Hero", HexColor::Turquoise } },
        { 24107, { "Mad Scientist", HexColor::DodgerBlue } },
        { 24108, { "Artisan", HexColor::MediumSlateBlue } },
        { 24109, { "Staff Member", HexColor::DarkMagenta } },
        { 24110, { "Game Master", HexColor::DarkGoldenRod } },
        { 24114, { "Content Creator", HexColor::RosyBrown } }
    };

    CTexture* kill0 = new CTexture{};
    CTexture* kill1 = new CTexture{};
    CTexture* kill2 = new CTexture{};
    CTexture* kill3 = new CTexture{};
    CTexture* kill4 = new CTexture{};
    CTexture* kill5 = new CTexture{};
    CTexture* kill6 = new CTexture{};
    CTexture* kill7 = new CTexture{};
    CTexture* kill8 = new CTexture{};
    CTexture* adminTex = new CTexture{};
    CTexture* vipTex = new CTexture{};

    // Aggiungi altre immagini per gif qui!

    CTexture* admin0 = new CTexture{};
    CTexture* admin1 = new CTexture{};
    CTexture* admin2 = new CTexture{};
    CTexture* admin3 = new CTexture{};
    CTexture* admin4 = new CTexture{};
    CTexture* admin5 = new CTexture{};
    CTexture* admin6 = new CTexture{};
    CTexture* admin7 = new CTexture{};
    CTexture* admin8 = new CTexture{};
    CTexture* admin9 = new CTexture{};
    CTexture* admin10 = new CTexture{};
    CTexture* admin11 = new CTexture{};
    CTexture* admin12 = new CTexture{};
    CTexture* admin13 = new CTexture{};
    CTexture* admin14 = new CTexture{};
    CTexture* admin15 = new CTexture{};
    CTexture* admin16 = new CTexture{};
    CTexture* admin17 = new CTexture{};
    CTexture* admin18 = new CTexture{};
    CTexture* admin19 = new CTexture{};

    GifAnimator adminAnimator(100); // 100ms per frame

    void InitiateTitles() {
        auto clear = [](CTexture* tex) {
            tex->texture = (LPDIRECT3DTEXTURE9)0x0;
            memset(tex->pad24, 0, sizeof(tex->pad24));
            tex->size.width = 0.0f;
            tex->size.height = 0.0f;
        };
        clear(kill0); clear(kill1); clear(kill2); clear(kill3); clear(kill4);
        clear(kill5); clear(kill6); clear(kill7); clear(kill8); clear(adminTex);
        clear(vipTex);
        clear(admin0); clear(admin1); clear(admin2); clear(admin3); clear(admin4);
        clear(admin5); clear(admin6); clear(admin7); clear(admin8); clear(admin9);
        clear(admin10); clear(admin11); clear(admin12); clear(admin13); clear(admin14);
        clear(admin15); clear(admin16); clear(admin17); clear(admin18); clear(admin19);

        // aggiungi le immagini per gif qui! sopra su "clear" e sotto su "frames"

        std::vector<CTexture*> frames = { admin0, admin1, admin2, admin3, admin4, admin5, admin6, admin7, admin8, admin9, admin10, admin11, admin12, admin13, admin14, admin15, admin16, admin17, admin18, admin19 };
        adminAnimator.setFrames(frames);
    }

    void hook(CCharacter* user, float x, float y, float extrusion) {
        const char* text = nullptr;
        HexColor color = HexColor::Gold;
        CTexture* selectedTexture = nullptr;

		// Aggiungi altre immagini per gif qui!

        CTexture::CreateFromFile(adminTex, "data/interface/title", "admin.tga", 220, 64);
        CTexture::CreateFromFile(vipTex, "data/interface/title", "vip.tga", 220, 64);
        CTexture::CreateFromFile(kill0, "data/interface/title", "kill.tga", 220, 64);
        CTexture::CreateFromFile(kill1, "data/interface/title", "kill1.tga", 220, 64);
        CTexture::CreateFromFile(kill2, "data/interface/title", "kill2.tga", 220, 64);
        CTexture::CreateFromFile(kill3, "data/interface/title", "kill3.tga", 220, 64);
        CTexture::CreateFromFile(kill4, "data/interface/title", "kill4.tga", 220, 64);
        CTexture::CreateFromFile(kill5, "data/interface/title", "kill5.tga", 220, 64);
        CTexture::CreateFromFile(kill6, "data/interface/title", "kill6.tga", 220, 64);
        CTexture::CreateFromFile(kill7, "data/interface/title", "kill7.tga", 220, 64);
        CTexture::CreateFromFile(kill8, "data/interface/title", "kill8.tga", 220, 64);
        CTexture::CreateFromFile(admin0, "data/interface/title", "admin0.tga", 220, 64);
        CTexture::CreateFromFile(admin1, "data/interface/title", "admin1.tga", 220, 64);
        CTexture::CreateFromFile(admin2, "data/interface/title", "admin2.tga", 220, 64);
        CTexture::CreateFromFile(admin3, "data/interface/title", "admin3.tga", 220, 64);
        CTexture::CreateFromFile(admin4, "data/interface/title", "admin4.tga", 220, 64);
        CTexture::CreateFromFile(admin5, "data/interface/title", "admin5.tga", 220, 64);
        CTexture::CreateFromFile(admin6, "data/interface/title", "admin6.tga", 220, 64);
        CTexture::CreateFromFile(admin7, "data/interface/title", "admin7.tga", 220, 64);
        CTexture::CreateFromFile(admin8, "data/interface/title", "admin8.tga", 220, 64);
        CTexture::CreateFromFile(admin9, "data/interface/title", "admin9.tga", 220, 64);
        CTexture::CreateFromFile(admin10, "data/interface/title", "admin10.tga", 220, 64);
        CTexture::CreateFromFile(admin11, "data/interface/title", "admin11.tga", 220, 64);
        CTexture::CreateFromFile(admin12, "data/interface/title", "admin12.tga", 220, 64);
        CTexture::CreateFromFile(admin13, "data/interface/title", "admin13.tga", 220, 64);
        CTexture::CreateFromFile(admin14, "data/interface/title", "admin14.tga", 220, 64);
        CTexture::CreateFromFile(admin15, "data/interface/title", "admin15.tga", 220, 64);
        CTexture::CreateFromFile(admin16, "data/interface/title", "admin16.tga", 220, 64);
        CTexture::CreateFromFile(admin17, "data/interface/title", "admin17.tga", 220, 64);
        CTexture::CreateFromFile(admin18, "data/interface/title", "admin18.tga", 220, 64);
        CTexture::CreateFromFile(admin19, "data/interface/title", "admin19.tga", 220, 64);

        if (user->isAdmin) {
            text = "";
            selectedTexture = adminAnimator.getCurrentFrame();
            if (!selectedTexture) selectedTexture = adminTex;
        }
        else if (g_pPlayerData && g_pPlayerData->points >= 2000) {
            selectedTexture = vipTex;
        }
        else if (user->kills >= 7000000) selectedTexture = kill0;
        else if (user->kills >= 6500000) selectedTexture = kill1;
        else if (user->kills >= 6000000) selectedTexture = kill2;
        else if (user->kills >= 5500000) selectedTexture = kill3;
        else if (user->kills >= 5000000) selectedTexture = kill4;
        else if (user->kills >= 4000000) selectedTexture = kill5;
        else if (user->kills >= 3000000) selectedTexture = kill6;
        else if (user->kills >= 2000000) selectedTexture = kill7;
        else if (user->kills > 1000000)  selectedTexture = kill8;

        if (!selectedTexture && user->mantleType && user->mantleTypeId) {
            auto itemInfo = CDataFile::GetItemInfo(user->mantleType, user->mantleTypeId);
            if (itemInfo) {
                auto itemId = (itemInfo->type * 1000) + itemInfo->typeId;
                auto it = items.find(itemId);
                if (it != items.end()) {
                    text = std::get<0>(it->second);
                    color = std::get<1>(it->second);
                }
            }
        }

        if (user->title.text) {
            user->title.text->texture->Release();
            user->title.text = nullptr;
        }
        if (text) {
            user->title.text = CStaticText::Create(text);
            auto w = CStaticText::GetTextWidth(text);
            user->title.pointX = int(w * 0.5);
        }

        if (!text && !selectedTexture)
            return;

        auto imageWidth = selectedTexture && selectedTexture->texture ? selectedTexture->size.width : 0.0f;
        auto posX = x - (imageWidth / 2.0f) + 30.0f;
        auto posY = y - 80.0f;

        if (selectedTexture && selectedTexture->texture)
            CTexture::Render(selectedTexture, long(posX), long(posY), 0.10F);

        if (user->title.text) {
            auto textWidth = CStaticText::GetTextWidth(text);
            auto textPosX = selectedTexture ?
                posX + (imageWidth / 2.0f) - (textWidth / 2.0f) :
                x - (textWidth / 2.0f);

            CStaticText::Draw(
                user->title.text,
                long(textPosX),
                long(posY + 30),
                extrusion,
                std::to_underlying(color)
            );
        }
    }

    void reset(CCharacter* user) {
        if (!user->title.text)
            return;
        user->title.text->texture->Release();
        user->title.text = nullptr;
    }
} // namespace title

unsigned u0x453E81 = 0x453E81;
void __declspec(naked) naked_0x453E7C()
{
    __asm
    {
        pushad
        pushfd

        sub esp, 0xC
        fld dword ptr[esp + 0x4C]
        fstp dword ptr[esp + 0x8]

        fld dword ptr[esp + 0x48]
        fstp dword ptr[esp + 0x4]

        fld dword ptr[esp + 0x44]
        fstp dword ptr[esp]

            push esi // user
                call title::hook
                add esp, 0x10

                popfd
                popad

                // original
                mov eax, dword ptr ds : [0x22B69A8]
                jmp u0x453E81
    }
}

unsigned n0x4184CF = 0x4184CF;
unsigned u0x418312 = 0x418312;
void __declspec(naked) naked_0x41830D()
{
    __asm
    {
        // monster->model
        cmp dword ptr[eax + 0x74], 0x0
        je _0x4184CF

        // original
        cmp dword ptr[esp + 0x38], 0x0
        jmp u0x418312

        _0x4184CF :
        jmp n0x4184CF
    }
}

unsigned u0x412765 = 0x412765;
void __declspec(naked) naked_0x41275F()
{
    __asm
    {
        fld dword ptr[title::chat_y_add]
        jmp u0x412765
    }
}

unsigned u0x59F0C8 = 0x59F0C8;
void __declspec(naked) naked_0x59F0C3()
{
    __asm
    {
        pushad

        push esi
        call title::reset
        add esp, 0x4

        popad

        // original 
        cmp byte ptr[esp + 0x14], 0x0
        jmp u0x59F0C8
    }
}

void hook::title()
{
    util::detour((void*)0x453E7C, naked_0x453E7C, 5);
    // hide pets without a model
    util::detour((void*)0x41830D, naked_0x41830D, 5);
    // increase chat balloon height (1.5 to 1.75)
    util::detour((void*)0x41275F, naked_0x41275F, 6);
    // 0x507 packet method
    util::detour((void*)0x59F0C3, naked_0x59F0C3, 5);
    //initiate titles
    CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(title::InitiateTitles), NULL, 0, 0);
}

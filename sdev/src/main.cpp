#include "include/main.h"
#include "include/shaiya/include/CCharacter.h"
using namespace shaiya;

void user_ctor_hook(CCharacter* user)
{
    user->title.text = nullptr;
    user->title.pointX = 0;
}

unsigned u0x419E79 = 0x419E79;
void __declspec(naked) naked_0x419E73()
{
    __asm
    {
        // original
        mov [esi+0x434],ebx

        pushad

        push esi // user
        call user_ctor_hook
        add esp,0x4

        popad

        jmp u0x419E79
    }
}

void Main()
{
    hook::camera_limit();
    hook::character();
    hook::cmd();
    hook::custom_game();
    hook::equipment();
    hook::gui();
    hook::item_icon();
    hook::name_color();
    hook::packet();
    hook::quick_slot();
    hook::title();
    hook::vehicle();
    hook::window();
}

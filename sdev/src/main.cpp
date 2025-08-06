#include "include/main.h"

// NUOVE FUNZIONI
#include "include/shaiya/include/CWindow.h"
#include "include/shaiya/include/IDmobnpc.h"
#include "include/shaiya/include/buff_transparency.h"

void Main()
{
    // Attiva il cheat
    EnableCheat(); // Ora il compilatore riconosce questa funzione

    hook::camera_limit();
    hook::character();
    hook::command();
    hook::custom_game();
    hook::equipment();
    hook::exp_view();
    hook::input();
    hook::item_icon();
    hook::name_color();
    hook::packet();
    hook::patch();
    hook::quick_slot();
    hook::title();
    hook::vehicle();
    hook::weapon_step();
    hook::window();

    // NUOVE FUNZIONI
    hook::stats_color();
    hook::recreationviewer();
    hook::buff_transparency();
    shaiya::CWindow::UpdateWindowTitle(); // Avvia l'aggiornamento del titolo della finestra
}

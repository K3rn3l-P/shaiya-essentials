#include "include/shaiya/include/CWindow.h"

// NUOVA FUNZIONE
#include <windows.h>
#include <string>
#include <thread>
#include <chrono>

using namespace shaiya;

// NUOVA FUNZIONE
// Valori globali per la funzione UpdateWindowTitle
auto CharName = reinterpret_cast<char*>(0x09144CE); // Modifica indirizzo se necessario
const char* window_class_name = "GAME";            // Nome della finestra trovato con Spy++
const char* window_title_format = "Shaiya Duff";   // Formato del titolo della finestra

// NUOVA FUNZIONE
// Nuova funzione: Aggiorna il titolo della finestra
void CWindow::UpdateWindowTitle()
{
    std::thread([]() {
        while (true) {
            HWND hwnd = FindWindowA(window_class_name, nullptr); // Trova la finestra usando il nome della classe
            if (!hwnd) {
                // Se la finestra non è trovata, riprova dopo 3 secondi
                std::this_thread::sleep_for(std::chrono::seconds(3));
                continue;
            }

            // Aggiorna il titolo della finestra in base al valore di CharName
            if (strlen(CharName) > 1) {
                SetWindowTextA(hwnd, (std::string(window_title_format) + " - Playing as " + CharName).c_str());
            }
            else {
                SetWindowTextA(hwnd, (std::string(window_title_format) + " ").c_str());
            }

            // Attendi 3 secondi prima di aggiornare di nuovo
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
        }).detach();
}

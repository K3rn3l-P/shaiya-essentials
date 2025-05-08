#include "include/shaiya/include/CWindow.h"

// NUOVE FUNZIONI
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <string>
#include <thread>
#include <chrono>
#include <WinSock2.h>
#include <WS2tcpip.h> // Per inet_pton
#include <iostream>

using namespace shaiya;

// NUOVE FUNZIONI
// Valori globali per la funzione UpdateWindowTitle
auto CharName = reinterpret_cast<char*>(0x09144CE); // Modifica indirizzo se necessario
const char* window_class_name = "GAME";            // Nome della finestra trovato con Spy++
const char* window_title_format = "Shaiya Duff";   // Formato del titolo della finestra

void CWindow::Draw(LPDIRECT3DBASETEXTURE9 texture, int x, int y)
{
    typedef void(__thiscall* LPFN)(LPDIRECT3DBASETEXTURE9, int, int);
    (*(LPFN)0x57B860)(texture, x, y);
}

// NUOVE FUNZIONI
int CWindow::get_ping_and_packet_loss(const char* server_address, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return -1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return -1;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, server_address, &server.sin_addr);

    auto start = std::chrono::steady_clock::now();

    if (connect(sock, (SOCKADDR*)&server, sizeof(server)) != 0) {
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    closesocket(sock);
    WSACleanup();

    // Converti esplicitamente la durata a int
    return static_cast<int>(duration.count());
}

int CWindow::calculate_packet_loss(int num_attempts, int packet_loss) {
    return (packet_loss * 100);
}

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

            int ping = CWindow::get_ping_and_packet_loss("100.95.179.88", 80);
            int packet_loss = CWindow::calculate_packet_loss(10, 0);

            // Aggiorna il titolo della finestra in base al valore di CharName
            if (strlen(CharName) > 1) {
                SetWindowTextA(hwnd, (std::string(window_title_format) + " - Playing as " + CharName + " | Ping: " + std::to_string(ping) + "ms - Packet Loss: " + std::to_string(packet_loss) + "%").c_str());
            }
            else {
                SetWindowTextA(hwnd, (std::string(window_title_format) + " - No character selected | Ping: " + std::to_string(ping) + "ms - Packet Loss: " + std::to_string(packet_loss) + "%").c_str());
            }

            // Attendi 3 secondi prima di aggiornare di nuovo
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
        }).detach();
}
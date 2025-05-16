#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <chrono>
#include <cwchar>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <ctime>
#include <vector>
#include "include/main.h"
#include "AntiCheatEnhanced.cpp"  // o meglio se come .h/.cpp separati

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "ws2_32.lib")

std::mutex g_logMutex;

extern "C" __declspec(dllexport) void DllExport() {}

std::wstring GetCurrentTimeString() {
    std::time_t now = std::time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);

    wchar_t buffer[80];
    std::wcsftime(buffer, sizeof(buffer) / sizeof(wchar_t), L"%H:%M:%S", &timeinfo);
    return buffer;
}

void Log(const std::wstring& message) {
    std::lock_guard<std::mutex> lock(g_logMutex);
    std::wofstream logfile("anticheat.log", std::ios_base::app);
    if (logfile.is_open()) {
        logfile << L"[" << GetCurrentTimeString() << L"] " << message << std::endl;
    }
}

bool IsSystemProcess(const std::wstring& name) {
    return name == L"[System Process]" || name == L"System";
}

bool ContainsSuspiciousName(const std::wstring& name) {
    if (IsSystemProcess(name)) {
        return false;
    }

    const wchar_t* patterns[] = {
    L"cheatengine", L"cheat engine", L"x64dbg", L"x32dbg", L"ollydbg",
    L"ida", L"scylla", L"reclass", L"artmoney", L"wpe pro",          // Aggiunto "ida"
    L"process hacker", L"processhacker", L"gamehack", L"speedhack",
    L"hxd", L"mhxd", L"hexedit", L"winhex"                          // Aggiunti tool hex
    };

    std::wstring lname = name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::towlower);

    for (const auto& pattern : patterns) {
        if (lname.find(pattern) != std::wstring::npos) {
            Log(L"Match valido trovato: " + name + L" | Pattern: " + pattern);
            return true;
        }
    }
    return false;
}

bool IsSuspiciousDllLoaded() {
    HMODULE hMods[1024];
    DWORD cbNeeded;
    HANDLE hProcess = GetCurrentProcess();

    Log(L"Controllo moduli caricati...");
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (size_t i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
            wchar_t szModName[MAX_PATH];
            if (GetModuleBaseNameW(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(wchar_t))) {
                std::wstring modName(szModName);
                Log(L"Modulo trovato: " + modName);
                if (ContainsSuspiciousName(modName)) {
                    Log(L"Modulo sospetto rilevato: " + modName);
                    return true;
                }
            }
        }
    }
    return false;
}

bool IsCheatToolRunning() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        Log(L"Errore creazione snapshot");
        return false;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    Log(L"Scan processi in corso...");
    if (Process32FirstW(snapshot, &pe32)) {
        do {
            std::wstring procName(pe32.szExeFile);
            if (IsSystemProcess(procName)) {
                continue;
            }

            Log(L"Processo trovato: " + procName);
            if (ContainsSuspiciousName(procName)) {
                Log(L"Processo sospetto rilevato: " + procName);
                CloseHandle(snapshot);
                return true;
            }
        } while (Process32NextW(snapshot, &pe32));
    }
    CloseHandle(snapshot);
    return false;
}

bool VerifyNetwork() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        Log(L"Errore WSAStartup");
        return false;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        Log(L"Errore creazione socket");
        WSACleanup();
        return false;
    }

    sockaddr_in srv{};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(443);
    InetPtonW(AF_INET, L"8.8.8.8", &srv.sin_addr);

    bool ok = connect(sock, (sockaddr*)&srv, sizeof(srv)) == 0;
    if (!ok) Log(L"Connessione a 8.8.8.8:443 fallita");

    closesocket(sock);
    WSACleanup();
    return ok;
}

bool VerifySelfChecksum() {
    const std::wstring exePath = L"Game-DEV.exe"; // Sostituire con nome reale
    HANDLE hFile = CreateFileW(exePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        Log(L"Impossibile aprire l'eseguibile");
        return false;
    }

    DWORD size = GetFileSize(hFile, nullptr);
    std::vector<BYTE> buffer(size);
    DWORD read = 0;

    if (!ReadFile(hFile, buffer.data(), size, &read, nullptr) || read != size) {
        Log(L"Errore lettura file");
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);

    uint32_t sum = 0;
    for (BYTE b : buffer) sum += b;

    const uint32_t expected = 0x1833A394; // Sostituire con checksum reale
    if (sum != expected) Log(L"Checksum mismatch: " + std::to_wstring(sum));

    return sum == expected;
}

bool IsDebuggerPresentAdvanced() {
    BOOL isDebuggerPresent = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebuggerPresent);
    return isDebuggerPresent || IsDebuggerPresent();
}

// Aggiungi questa nuova funzione per controllare processi specifici
bool IsProcessRunning(const wchar_t* processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(snapshot, &pe32)) {
        do {
            std::wstring procName(pe32.szExeFile);
            if (procName.find(processName) != std::wstring::npos) {
                CloseHandle(snapshot);
                return true;
            }
        } while (Process32NextW(snapshot, &pe32));
    }
    CloseHandle(snapshot);
    return false;
}

// Modifica la funzione IsRunningInSandbox
bool IsRunningInSandbox() {
    const wchar_t* sandboxProcesses[] = {
        L"vboxservice", L"vboxtray", L"vmwaretray",
        L"xenservice", L"qemu-ga", L"prl_cc"
    };

    for (const auto& proc : sandboxProcesses) {
        if (IsProcessRunning(proc)) {  // Chiamata corretta con un solo argomento
            Log(L"Sandbox rilevata: " + std::wstring(proc));
            return true;
        }
    }
    return false;
}

bool IsSuspiciousBehavior() {
    if (!VerifySelfChecksum()) {
        Log(L"Checksum mismatch");
        return true;
    }
    if (!VerifyNetwork()) {
        Log(L"Network verification failed");
        return true;
    }
    if (IsDebuggerPresentAdvanced()) {
        Log(L"Debugger detected");
        return true;
    }
    return false;
}

DWORD WINAPI MessageBoxThread(LPVOID) {
    MessageBoxW(NULL, L"AntiCheat: Cheat tool or DLL detected. Closing game.", L"AntiCheat", MB_ICONERROR | MB_SYSTEMMODAL);
    return 0;
}

DWORD WINAPI AntiCheatThread(LPVOID) {
    Log(L"Avvio thread AntiCheat");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    while (true) {
        Log(L"Scan ciclo iniziato");
        bool cheatProcess = IsCheatToolRunning();
        bool cheatDll = IsSuspiciousDllLoaded();
        bool suspiciousBehavior = IsSuspiciousBehavior();

        if (RunAdvancedChecks()) {
            Log(L"Rilevato comportamento sospetto avanzato (hook/thread/patch)");
            TerminateProcess(GetCurrentProcess(), 1);
        }

        if (cheatProcess || cheatDll || suspiciousBehavior) {
            Log(L"Rilevazione cheat! Process: " + std::to_wstring(cheatProcess) +
                L" DLL: " + std::to_wstring(cheatDll) +
                L" Behavior: " + std::to_wstring(suspiciousBehavior));

            // Mostra la message box e attendi il rendering
            HANDLE hThread = CreateThread(nullptr, 0, MessageBoxThread, nullptr, 0, nullptr);
            WaitForSingleObject(hThread, 500);  // Aspetta fino a 500ms per l'apertura

            // Forza la scrittura del log
            {
                std::wofstream logfile("anticheat.log", std::ios_base::app);
                logfile << L"[FORCED TERMINATION]" << std::endl;
            }

            TerminateProcess(GetCurrentProcess(), 1);  // Termina dopo la visualizzazione
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, AntiCheatThread, NULL, 0, NULL); // Aggiungi questa linea
        Main();
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
    case DLL_THREAD_ATTACH:
        break;
    }
    return TRUE;
}

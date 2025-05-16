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
#include <queue>
#include <cmath>
#include <numeric>


#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "ws2_32.lib")

extern "C" __declspec(dllexport) void DllExport() {}

/*std::mutex g_logMutex;

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
}*/

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
    L"hxd", L"mhxd", L"hexedit", L"injector", L"delite", L"winhex"              // Aggiunti tool hex
    };

    std::wstring lname = name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::towlower);

    for (const auto& pattern : patterns) {
        if (lname.find(pattern) != std::wstring::npos) {
            //Log(L"Match valido trovato: " + name + L" | Pattern: " + pattern);
            return true;
        }
    }
    return false;
}

// Aggiungi queste costanti in cima al file
const int KEY_SAMPLES = 15;         // Numero di campioni per il rilevamento
const double MACRO_THRESHOLD = 30.0; // Soglia in millisecondi (30ms tra pressioni = 33 pressioni/secondo)
const double HUMAN_VARIANCE = 8.0;  // Deviazione minima per input umano

// Struttura per tracciamento input
struct KeyEvent {
    DWORD key;
    DWORD timestamp;
};

std::queue<KeyEvent> keyEvents;
std::mutex keyMutex;

// Hook per la tastiera (solo logica, non hook effettivo)
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* keyInfo = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            std::lock_guard<std::mutex> lock(keyMutex);

            // Mantieni solo gli ultimi KEY_SAMPLES eventi
            if (keyEvents.size() >= KEY_SAMPLES) {
                keyEvents.pop();
            }

            keyEvents.push({ keyInfo->vkCode, GetTickCount() });
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Calcola la deviazione standard degli intervalli
double CalculateVariance(const std::vector<DWORD>& intervals) {
    if (intervals.size() < 2) return 0.0;

    double mean = std::accumulate(intervals.begin(), intervals.end(), 0.0) / intervals.size();

    // Usa una funzione lambda per il calcolo della varianza
    double variance = std::accumulate(intervals.begin(), intervals.end(), 0.0,
        [mean](double acc, DWORD interval) {
            return acc + std::pow(interval - mean, 2);
        });

    return std::sqrt(variance / intervals.size());
}

// Controllo macro avanzato
bool IsMacroDetected() {
    std::lock_guard<std::mutex> lock(keyMutex);

    if (keyEvents.size() < KEY_SAMPLES) return false;

    std::vector<DWORD> intervals;
    DWORD prevTime = keyEvents.front().timestamp;

    while (!keyEvents.empty()) {
        DWORD currentTime = keyEvents.front().timestamp;
        keyEvents.pop();

        if (prevTime != 0) {
            intervals.push_back(currentTime - prevTime);
        }
        prevTime = currentTime;
    }

    // Calcola statistiche
    double avgSpeed = std::accumulate(intervals.begin(), intervals.end(), 0.0) / intervals.size();
    double variance = CalculateVariance(intervals);

    // 1. Velocità sovrumana
    bool speedFlag = avgSpeed < MACRO_THRESHOLD;

    // 2. Regolarità meccanica
    bool regularityFlag = variance < HUMAN_VARIANCE;

    // 3. Combinazione pericolosa
    return speedFlag && regularityFlag;
}

bool IsSuspiciousDllLoaded() {
    HMODULE hMods[1024];
    DWORD cbNeeded;
    HANDLE hProcess = GetCurrentProcess();

    //Log(L"Controllo moduli caricati...");
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (size_t i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
            wchar_t szModName[MAX_PATH];
            if (GetModuleBaseNameW(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(wchar_t))) {
                std::wstring modName(szModName);
                //Log(L"Modulo trovato: " + modName);
                if (ContainsSuspiciousName(modName)) {
                    //Log(L"Modulo sospetto rilevato: " + modName);
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
        //Log(L"Errore creazione snapshot");
        return false;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    //Log(L"Scan processi in corso...");
    if (Process32FirstW(snapshot, &pe32)) {
        do {
            std::wstring procName(pe32.szExeFile);
            if (IsSystemProcess(procName)) {
                continue;
            }

            //Log(L"Processo trovato: " + procName);
            if (ContainsSuspiciousName(procName)) {
                //Log(L"Processo sospetto rilevato: " + procName);
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
        //Log(L"Errore WSAStartup");
        return false;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        //Log(L"Errore creazione socket");
        WSACleanup();
        return false;
    }

    sockaddr_in srv{};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(443);
    InetPtonW(AF_INET, L"8.8.8.8", &srv.sin_addr);

    bool ok = connect(sock, (sockaddr*)&srv, sizeof(srv)) == 0;
    if (!ok) //Log(L"Connessione a 8.8.8.8:443 fallita");

    closesocket(sock);
    WSACleanup();
    return ok;
}

bool VerifySelfChecksum() {
    const std::wstring exePath = L"Game.exe";
    HANDLE hFile = CreateFileW(exePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD size = GetFileSize(hFile, nullptr);
    std::vector<BYTE> buffer(size);
    DWORD read = 0;

    if (!ReadFile(hFile, buffer.data(), size, &read, nullptr) || read != size) {
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);

    uint32_t sum = 0;
    for (BYTE b : buffer) sum += b;

    const uint32_t expected = 0x1833A77E; // Cambia nel tuo codice
    return (sum == expected);  
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
        L"xenservice", L"qemu-ga", L"prl_cc", L"sandboxie"
    };

    for (const auto& proc : sandboxProcesses) {
        if (IsProcessRunning(proc)) {  // Chiamata corretta con un solo argomento
            //Log(L"Sandbox rilevata: " + std::wstring(proc));
            return true;
        }
    }
    return false;
}

bool IsSuspiciousBehavior() {
    if (!VerifySelfChecksum()) {
        //Log(L"Checksum mismatch");
        return true;
    }
    if (!VerifyNetwork()) {
        //Log(L"Network verification failed");
        return true;
    }
    if (IsDebuggerPresentAdvanced()) {
        //Log(L"Debugger detected");
        return true;
    }
    return false;
}

DWORD WINAPI MessageBoxThread(LPVOID) {
    MessageBoxW(NULL, L"Security violation detected. The game will now close.", L"Anti-Cheat System", MB_ICONERROR | MB_SYSTEMMODAL);
    return 0;
}

DWORD WINAPI AntiCheatThread(LPVOID) {
    //Log(L"Avvio thread AntiCheat");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, NULL, 0);


    while (true) {
        //Log(L"Scan ciclo iniziato");
        bool cheatProcess = IsCheatToolRunning();
        bool cheatDll = IsSuspiciousDllLoaded();
        bool suspiciousBehavior = IsSuspiciousBehavior();
        // Controllo macro
        if (IsMacroDetected()) {
            TerminateProcess(GetCurrentProcess(), 1);
        }

        if (RunAdvancedChecks()) {
            //Log(L"Rilevato comportamento sospetto avanzato (hook/thread/patch)");
            TerminateProcess(GetCurrentProcess(), 1);
        }

        if (cheatProcess || cheatDll || suspiciousBehavior) {
            /*
            Log(L"Rilevazione cheat! Process: " + std::to_wstring(cheatProcess) +
                L" DLL: " + std::to_wstring(cheatDll) +
                L" Behavior: " + std::to_wstring(suspiciousBehavior));
            */

            HANDLE hThread = CreateThread(nullptr, 0, MessageBoxThread, nullptr, 0, nullptr);
            WaitForSingleObject(hThread, 500);

            TerminateProcess(GetCurrentProcess(), 1);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    UnhookWindowsHookEx(hook);
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

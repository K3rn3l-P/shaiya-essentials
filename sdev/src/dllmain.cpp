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
#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")
#include "../../UltimateAntiCheat/Common/sha256_hashes.hpp"

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

// Modifica la struttura e le dichiarazioni globali
struct KeyEvent {
    DWORD key;
    DWORD timestamp;  // Manteniamo DWORD per compatibilit� con GetTickCount()
    bool isSystemKey;
};

std::queue<KeyEvent> keyEvents;  // Dichiarazione esplicita
std::mutex keyMutex;             // Dichiarazione esplicita

// Aggiorna la funzione CalculateVariance per usare DWORD
double CalculateVariance(const std::vector<DWORD>& intervals) {
    if (intervals.size() < 2) return 0.0;

    double mean = std::accumulate(intervals.begin(), intervals.end(), 0.0) / intervals.size();

    double variance = std::accumulate(intervals.begin(), intervals.end(), 0.0,
        [mean](double acc, DWORD interval) {
            return acc + std::pow(static_cast<double>(interval) - mean, 2);
        });

    return std::sqrt(variance / intervals.size());
}

// Modifica KeyboardHook per usare GetTickCount() con controllo overflow
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* keyInfo = (KBDLLHOOKSTRUCT*)lParam;

        // Ignora i tasti di sistema (CTRL, ALT, SHIFT, etc.)
        bool isSystemKey = (keyInfo->vkCode == VK_CONTROL ||
            keyInfo->vkCode == VK_MENU ||
            keyInfo->vkCode == VK_SHIFT ||
            keyInfo->vkCode == VK_LWIN ||
            keyInfo->vkCode == VK_RWIN);

        if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && !isSystemKey) {
            std::lock_guard<std::mutex> lock(keyMutex);

            if (keyEvents.size() >= KEY_SAMPLES) {
                keyEvents.pop();
            }

            keyEvents.push({ keyInfo->vkCode, GetTickCount(), isSystemKey });
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Aggiorna IsMacroDetected
bool IsMacroDetected() {
    std::lock_guard<std::mutex> lock(keyMutex);

    if (keyEvents.size() < KEY_SAMPLES) return false;

    std::vector<DWORD> intervals;
    DWORD prevTime = keyEvents.front().timestamp;

    // Creiamo una copia temporanea per non svuotare la coda originale
    auto tempQueue = keyEvents;
    while (!tempQueue.empty()) {
        DWORD currentTime = tempQueue.front().timestamp;
        tempQueue.pop();

        if (prevTime != 0) {
            // Gestione overflow di GetTickCount()
            DWORD interval = (currentTime >= prevTime) ?
                (currentTime - prevTime) :
                ((0xFFFFFFFF - prevTime) + currentTime);
            intervals.push_back(interval);
        }
        prevTime = currentTime;
    }

    double avgSpeed = std::accumulate(intervals.begin(), intervals.end(), 0.0) / intervals.size();
    double variance = CalculateVariance(intervals);

    bool speedFlag = avgSpeed < MACRO_THRESHOLD;
    bool regularityFlag = variance < HUMAN_VARIANCE;

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

std::wstring BytesToHexString(const BYTE* data, DWORD length) {
    std::wstringstream ss;
    ss << std::hex << std::uppercase;
    for (DWORD i = 0; i < length; ++i) {
        ss.width(2);
        ss.fill(L'0');
        ss << static_cast<int>(data[i]);
    }
    return ss.str();
}

std::wstring CalculateFileSHA256(const std::wstring& filePath) {
    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BYTE buffer[4096];
    DWORD bytesRead = 0;
    BYTE hash[32];  // SHA256 = 32 bytes
    DWORD hashLen = sizeof(hash);
    std::wstring hashStr;

    // Open the file
    hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return L"";

    if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        CloseHandle(hFile);
        return L"";
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        CloseHandle(hFile);
        return L"";
    }

    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead != 0) {
        if (!CryptHashData(hHash, buffer, bytesRead, 0)) {
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            CloseHandle(hFile);
            return L"";
        }
    }

    if (CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
        hashStr = BytesToHexString(hash, hashLen);
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    CloseHandle(hFile);

    return hashStr;
}


bool VerifyFileChecksum(const std::wstring& exePath, const std::wstring& expectedHash) {
    std::wstring actualHash = CalculateFileSHA256(exePath);
    if (actualHash.empty()) {
        return false;
    }
    std::transform(actualHash.begin(), actualHash.end(), actualHash.begin(), ::towupper);
    return (actualHash == expectedHash);
}

bool VerifySelfChecksum() {
    struct FileCheck {
        std::wstring exePath;
        std::wstring expectedHash;
    };

    // Inserisci qui gli hash reali di Uppdater.exe e game.exe
    const std::vector<FileCheck> filesToCheck = {
    {L"x32.exe",      std::wstring(expectedX32Sha256.begin(), expectedX32Sha256.end())},
    {L"Updater.exe",  std::wstring(expectedUpdaterSha256.begin(), expectedUpdaterSha256.end())},
    };

    for (const auto& file : filesToCheck) {
        if (!VerifyFileChecksum(file.exePath, file.expectedHash)) {
            return false; // Appena uno fallisce, esce subito
        }
    }
    return true; // Tutti i file sono validi
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
    // Riduci la priorit� del thread anti-cheat
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    //Log(L"Avvio thread AntiCheat");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, NULL, 0);


    while (true) {
        // Aggiungi un piccolo delay per ridurre l'impatto
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
            if (hThread != NULL) {
                WaitForSingleObject(hThread, 500);
                CloseHandle(hThread);  // Importante: chiudere l'handle dopo l'uso
            }
            TerminateProcess(GetCurrentProcess(), 1);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (hook) {
        UnhookWindowsHookEx(hook);
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
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
//#include "AntiCheatEnhanced.cpp"
#include <queue>
#include <cmath>
#include <numeric>
#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")
#include "../../UltimateAntiCheat/Common/sha256_hashes.hpp"

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "ws2_32.lib")

extern "C" __declspec(dllexport) void DllExport() {}

// ================= CHECKSUM INIZIO =================
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
    BYTE hash[32];
    DWORD hashLen = sizeof(hash);
    std::wstring hashStr;
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
    const std::vector<FileCheck> filesToCheck = {
    {L"x32.exe",      std::wstring(expectedX32Sha256.begin(), expectedX32Sha256.end())},
    {L"Updater.exe",  std::wstring(expectedUpdaterSha256.begin(), expectedUpdaterSha256.end())},
    };
    for (const auto& file : filesToCheck) {
        if (!VerifyFileChecksum(file.exePath, file.expectedHash)) {
            return false;
        }
    }
    return true;
}
// ================= CHECKSUM FINE =================

// ================= DEBUGGER INIZIO =================
bool IsDebuggerPresentAdvanced() {
    BOOL isDebuggerPresent = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebuggerPresent);
    return isDebuggerPresent || IsDebuggerPresent();
}
// ================= DEBUGGER FINE =================

// ================= PROCESSI INIZIO =================
bool IsSystemProcess(const std::wstring& name) {
    return name == L"[System Process]" || name == L"System";
}

bool ContainsSuspiciousName(const std::wstring& name) {
    if (IsSystemProcess(name)) {
        return false;
    }

    const wchar_t* patterns[] = {
    L"cheatengine", L"cheat engine", L"x64dbg", L"x32dbg", L"ollydbg",
    L"ida", L"scylla", L"reclass", L"artmoney", L"wpe pro",
    L"process hacker", L"processhacker", L"gamehack", L"speedhack",
    L"hxd", L"mhxd", L"hexedit", L"injector", L"delite", L"winhex"
    };

    std::wstring lname = name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::towlower);

    for (const auto& pattern : patterns) {
        if (lname.find(pattern) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

bool IsSuspiciousDllLoaded() {
    HMODULE hMods[1024];
    DWORD cbNeeded;
    HANDLE hProcess = GetCurrentProcess();

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (size_t i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
            wchar_t szModName[MAX_PATH];
            if (GetModuleBaseNameW(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(wchar_t))) {
                std::wstring modName(szModName);
                if (ContainsSuspiciousName(modName)) {
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
        return false;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(snapshot, &pe32)) {
        do {
            std::wstring procName(pe32.szExeFile);
            if (IsSystemProcess(procName)) {
                continue;
            }

            if (ContainsSuspiciousName(procName)) {
                CloseHandle(snapshot);
                return true;
            }
        } while (Process32NextW(snapshot, &pe32));
    }
    CloseHandle(snapshot);
    return false;
}

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

bool IsRunningInSandbox() {
    const wchar_t* sandboxProcesses[] = {
        L"vboxservice", L"vboxtray", L"vmwaretray",
        L"xenservice", L"qemu-ga", L"prl_cc", L"sandboxie"
    };

    for (const auto& proc : sandboxProcesses) {
        if (IsProcessRunning(proc)) {
            return true;
        }
    }
    return false;
}
// ================= PROCESSI FINE =================

// ================= MESSAGEBOX INIZIO =================
DWORD WINAPI MessageBoxThread(LPVOID) {
    MessageBoxW(NULL, L"Security violation detected. The game will now close.", L"Anti-Cheat System", MB_ICONERROR | MB_SYSTEMMODAL);
    return 0;
}
// ================= MESSAGEBOX FINE =================

// ================= ANTICHEAT THREAD INIZIO =================
DWORD WINAPI AntiCheatThread(LPVOID) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        bool cheatProcess = IsCheatToolRunning();
        bool cheatDll = IsSuspiciousDllLoaded();
        bool sandboxDetected = IsRunningInSandbox();
        if (cheatProcess || cheatDll || sandboxDetected) {
            HANDLE hThread = CreateThread(nullptr, 0, MessageBoxThread, nullptr, 0, nullptr);
            if (hThread != NULL) {
                WaitForSingleObject(hThread, 500);
                CloseHandle(hThread);
            }
            TerminateProcess(GetCurrentProcess(), 1);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
}
// ================= ANTICHEAT THREAD FINE =================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, AntiCheatThread, NULL, 0, NULL);
        Main();
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
    case DLL_THREAD_ATTACH:
        break;
    }
    return TRUE;
}


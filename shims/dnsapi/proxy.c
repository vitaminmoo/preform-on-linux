#include <windows.h>

#include "exports.h"

/*
 * Proxy for Wine's dnsapi.dll. Every export except the mDNS ones is an asm thunk
 * (thunks.S) that jumps through real[] into the real dnsapi in system32.
 */
void *real[NUM_THUNKS];

/* DnsStartMulticastQuery and DnsStopMulticastQuery live in mdns.c */
HMODULE real_dnsapi;

BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, void *p)
{
    if (reason == DLL_PROCESS_ATTACH) {
        WCHAR path[MAX_PATH];
        HMODULE m;
        GetSystemDirectoryW(path, MAX_PATH);
        lstrcatW(path, L"\\dnsapi.dll");
        if (!(m = real_dnsapi = LoadLibraryW(path))) return FALSE;
        for (int i = 0; i < NUM_THUNKS; i++) real[i] = (void *)GetProcAddress(m, names[i]);
    }
    return TRUE;
}

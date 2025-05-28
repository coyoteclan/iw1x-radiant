#include "pch.h"
#include "iw1x_core.h"

//extern "C" IW1X_API DWORD address_cgame_mp = 0;
//extern "C" IW1X_API DWORD address_ui_mp = 0;

void MSG_ERR(const char* msg)
{
    MessageBox(NULL, msg, MOD_NAME, MB_ICONERROR | MB_SETFOREGROUND);
}

bool RunningUnderWine() {
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll)
        return false;

    // Look for wine_get_version export
    FARPROC fn = GetProcAddress(hNtdll, "wine_get_version");
    return (fn != nullptr);
}

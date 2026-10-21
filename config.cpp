#include "config.h"
#include <windows.h>
#include <string>
#include <cstdio>

Config g_cfg;
extern HMODULE g_module;

static std::wstring iniPath() {
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(g_module, buf, MAX_PATH);
    std::wstring p(buf);
    return p.substr(0, p.find_last_of(L"\\/") + 1) + L"gdmenu.ini";
}

void configLoad() {
    auto p = iniPath();
    g_cfg.speedEnabled = GetPrivateProfileIntW(L"menu", L"speedEnabled", 0, p.c_str()) != 0;
    g_cfg.showFps      = GetPrivateProfileIntW(L"menu", L"showFps", 1, p.c_str()) != 0;
    wchar_t s[32];
    GetPrivateProfileStringW(L"menu", L"speed", L"1.0", s, 32, p.c_str());
    g_cfg.speed = (float)_wtof(s);
    if (g_cfg.speed < 0.05f || g_cfg.speed > 20.f) g_cfg.speed = 1.f;
}

void configSave() {
    auto p = iniPath();
    WritePrivateProfileStringW(L"menu", L"speedEnabled", g_cfg.speedEnabled ? L"1" : L"0", p.c_str());
    WritePrivateProfileStringW(L"menu", L"showFps", g_cfg.showFps ? L"1" : L"0", p.c_str());
    wchar_t s[32];
    swprintf(s, 32, L"%.3f", g_cfg.speed);
    WritePrivateProfileStringW(L"menu", L"speed", s, p.c_str());
}

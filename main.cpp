// Punto de entrada. La DLL se llama XInput9_1_0.dll: Geometry Dash la carga al arrancar,
// asi que no hace falta Geode ni ningun loader. Las 5 funciones reales se reenvian a la DLL del sistema.
#include <windows.h>
#include "MinHook.h"
#include "overlay.h"
#include "speedhack.h"
#include "config.h"

HMODULE g_module = nullptr;

#ifdef _MSC_VER
#pragma comment(linker, "/export:XInputGetState=C:\\Windows\\System32\\XInput9_1_0.XInputGetState")
#pragma comment(linker, "/export:XInputSetState=C:\\Windows\\System32\\XInput9_1_0.XInputSetState")
#pragma comment(linker, "/export:XInputGetCapabilities=C:\\Windows\\System32\\XInput9_1_0.XInputGetCapabilities")
#pragma comment(linker, "/export:XInputEnable=C:\\Windows\\System32\\XInput9_1_0.XInputEnable")
#pragma comment(linker, "/export:XInputGetDSoundAudioDeviceGuids=C:\\Windows\\System32\\XInput9_1_0.XInputGetDSoundAudioDeviceGuids")
#endif

static DWORD WINAPI initThread(LPVOID) {
    configLoad();
    if (MH_Initialize() != MH_OK) return 1;
    speedhack::install();
    overlay::install();
    MH_EnableHook(MH_ALL_HOOKS);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE h, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_module = h;
        DisableThreadLibraryCalls(h);
        CreateThread(nullptr, 0, initThread, nullptr, 0, nullptr);  // nunca trabajar pesado dentro de DllMain
    }
    return TRUE;
}

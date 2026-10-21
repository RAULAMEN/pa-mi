// Speedhack independiente de la version del juego: escala los relojes de Windows
// (QPC, GetTickCount, timeGetTime, hora del sistema) que el juego usa para calcular el delta time.
#include "speedhack.h"
#include <windows.h>
#include <mmsystem.h>
#include "MinHook.h"

namespace speedhack {

static SRWLOCK g_lock = SRWLOCK_INIT;
static double  g_speed = 1.0;

struct Clock { int64_t baseReal = 0, baseFake = 0; bool init = false; };
static Clock cQpc, cTick, cTick64, cTime, cFile, cFileP;

static int64_t scaleLocked(Clock& c, int64_t real) {
    if (!c.init) { c.baseReal = c.baseFake = real; c.init = true; }
    return c.baseFake + (int64_t)((double)(real - c.baseReal) * g_speed);
}
static int64_t scale(Clock& c, int64_t real) {
    AcquireSRWLockExclusive(&g_lock);
    int64_t r = scaleLocked(c, real);
    ReleaseSRWLockExclusive(&g_lock);
    return r;
}
static void rebase(Clock& c, int64_t real) {
    if (!c.init) return;
    c.baseFake = scaleLocked(c, real);
    c.baseReal = real;
}

// originales
static BOOL      (WINAPI* oQpc)(LARGE_INTEGER*) = nullptr;
static DWORD     (WINAPI* oTick)() = nullptr;
static ULONGLONG (WINAPI* oTick64)() = nullptr;
static DWORD     (WINAPI* oTime)() = nullptr;
static VOID      (WINAPI* oFile)(LPFILETIME) = nullptr;
static VOID      (WINAPI* oFileP)(LPFILETIME) = nullptr;

static int64_t ftToI64(const FILETIME& f) { return ((int64_t)f.dwHighDateTime << 32) | f.dwLowDateTime; }
static void i64ToFt(int64_t v, LPFILETIME f) { f->dwLowDateTime = (DWORD)v; f->dwHighDateTime = (DWORD)(v >> 32); }

static BOOL WINAPI hQpc(LARGE_INTEGER* out) {
    LARGE_INTEGER r; BOOL ok = oQpc(&r);
    if (ok) out->QuadPart = scale(cQpc, r.QuadPart);
    return ok;
}
static DWORD WINAPI hTick()       { return (DWORD)scale(cTick, (int64_t)oTick()); }
static ULONGLONG WINAPI hTick64() { return (ULONGLONG)scale(cTick64, (int64_t)oTick64()); }
static DWORD WINAPI hTime()       { return (DWORD)scale(cTime, (int64_t)oTime()); }
static VOID WINAPI hFile(LPFILETIME f)  { FILETIME r; oFile(&r);  i64ToFt(scale(cFile,  ftToI64(r)), f); }
static VOID WINAPI hFileP(LPFILETIME f) { FILETIME r; oFileP(&r); i64ToFt(scale(cFileP, ftToI64(r)), f); }

int64_t realQpc() { LARGE_INTEGER r; (oQpc ? oQpc : QueryPerformanceCounter)(&r); return r.QuadPart; }
int64_t realQpcFreq() { LARGE_INTEGER f; QueryPerformanceFrequency(&f); return f.QuadPart; }

void setSpeed(double speed) {
    if (speed < 0.01) speed = 0.01;
    AcquireSRWLockExclusive(&g_lock);
    if (oQpc)    { LARGE_INTEGER r; oQpc(&r); rebase(cQpc, r.QuadPart); }
    if (oTick)   rebase(cTick, oTick());
    if (oTick64) rebase(cTick64, (int64_t)oTick64());
    if (oTime)   rebase(cTime, oTime());
    if (oFile)   { FILETIME f; oFile(&f);  rebase(cFile,  ftToI64(f)); }
    if (oFileP)  { FILETIME f; oFileP(&f); rebase(cFileP, ftToI64(f)); }
    g_speed = speed;
    ReleaseSRWLockExclusive(&g_lock);
}

bool install() {
    LoadLibraryW(L"winmm.dll");
    bool ok = true;
    ok &= MH_CreateHookApi(L"kernel32", "QueryPerformanceCounter",          (void*)hQpc,   (void**)&oQpc)   == MH_OK;
    ok &= MH_CreateHookApi(L"kernel32", "GetTickCount",                     (void*)hTick,  (void**)&oTick)  == MH_OK;
    ok &= MH_CreateHookApi(L"kernel32", "GetTickCount64",                   (void*)hTick64,(void**)&oTick64)== MH_OK;
    ok &= MH_CreateHookApi(L"winmm",    "timeGetTime",                      (void*)hTime,  (void**)&oTime)  == MH_OK;
    ok &= MH_CreateHookApi(L"kernel32", "GetSystemTimeAsFileTime",          (void*)hFile,  (void**)&oFile)  == MH_OK;
    ok &= MH_CreateHookApi(L"kernel32", "GetSystemTimePreciseAsFileTime",   (void*)hFileP, (void**)&oFileP) == MH_OK;
    return ok;
}

} // namespace speedhack

// Menu Dear ImGui dibujado sobre el juego. Se engancha a wglSwapBuffers (OpenGL).
#include "overlay.h"
#include <windows.h>
#include <GL/gl.h>
#include "MinHook.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl2.h"
#include "config.h"
#include "speedhack.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace overlay {

static bool    g_init = false, g_open = false;
static HWND    g_hwnd = nullptr;
static WNDPROC g_origWndProc = nullptr;
static BOOL (WINAPI* oSwap)(HDC) = nullptr;

static LRESULT CALLBACK wndProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
    if (msg == WM_KEYDOWN && !(l & (1 << 30)) && (w == VK_TAB || w == VK_INSERT)) {
        g_open = !g_open;
        if (!g_open) configSave();
        return 0;
    }
    if (g_open) {
        ImGui_ImplWin32_WndProcHandler(h, msg, w, l);
        ImGuiIO& io = ImGui::GetIO();
        bool mouse = msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST;
        bool key   = (msg >= WM_KEYFIRST && msg <= WM_KEYLAST) || msg == WM_CHAR;
        if ((mouse && io.WantCaptureMouse) || (key && io.WantCaptureKeyboard)) return 0;
    }
    return CallWindowProcW(g_origWndProc, h, msg, w, l);
}

static void applySpeed() { speedhack::setSpeed(g_cfg.speedEnabled ? g_cfg.speed : 1.0); }

static void drawMenu() {
    ImGui::SetNextWindowSize(ImVec2(360, 220), ImGuiCond_FirstUseEver);
    ImGui::Begin("GDMenu  (TAB / INS para cerrar)");

    ImGui::SeparatorText("Velocidad");
    bool changed = ImGui::Checkbox("Speedhack", &g_cfg.speedEnabled);
    changed |= ImGui::SliderFloat("Multiplicador", &g_cfg.speed, 0.1f, 5.0f, "%.2fx", ImGuiSliderFlags_Logarithmic);
    if (changed) applySpeed();

    ImGui::SeparatorText("Interfaz");
    ImGui::Checkbox("Mostrar FPS", &g_cfg.showFps);

    // ---- AQUI van tus funciones que dependen de la version (noclip, etc.), ver README ----
    ImGui::End();
}

static void drawFps() {
    static int64_t last = speedhack::realQpc(); static int frames = 0; static float fps = 0;
    frames++;
    int64_t now = speedhack::realQpc();
    double dt = (double)(now - last) / (double)speedhack::realQpcFreq();
    if (dt >= 0.5) { fps = (float)(frames / dt); frames = 0; last = now; }
    char buf[32]; snprintf(buf, sizeof buf, "%.0f FPS", fps);
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(8, 6), IM_COL32(255, 255, 255, 220), buf);
}

static BOOL WINAPI hSwap(HDC hdc) {
    if (!g_init) {
        g_hwnd = WindowFromDC(hdc);
        if (g_hwnd) {
            ImGui::CreateContext();
            ImGui::GetIO().IniFilename = nullptr;
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_InitForOpenGL(g_hwnd);
            ImGui_ImplOpenGL2_Init();
            g_origWndProc = (WNDPROC)SetWindowLongPtrW(g_hwnd, GWLP_WNDPROC, (LONG_PTR)wndProc);
            g_init = true;
            applySpeed();
        }
    }
    if (g_init) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glMatrixMode(GL_PROJECTION); glPushMatrix();
        glMatrixMode(GL_MODELVIEW);  glPushMatrix();

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::GetIO().MouseDrawCursor = g_open;
        if (g_cfg.showFps) drawFps();
        if (g_open) drawMenu();
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW);  glPopMatrix();
        glPopClientAttrib();
        glPopAttrib();
    }
    return oSwap(hdc);
}

bool install() {
    HMODULE gl = GetModuleHandleW(L"opengl32.dll");
    if (!gl) gl = LoadLibraryW(L"opengl32.dll");
    if (!gl) return false;
    void* target = (void*)GetProcAddress(gl, "wglSwapBuffers");
    return target && MH_CreateHook(target, (void*)hSwap, (void**)&oSwap) == MH_OK;
}

} // namespace overlay

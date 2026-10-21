cmake_minimum_required(VERSION 3.21)
project(GDMenu LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")   # runtime estatico: no hace falta instalar nada

include(FetchContent)
# SOURCE_SUBDIR inexistente => solo descarga, no ejecuta sus CMakeLists
FetchContent_Declare(minhook GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git GIT_TAG v1.3.3 SOURCE_SUBDIR _none)
FetchContent_Declare(imgui   GIT_REPOSITORY https://github.com/ocornut/imgui.git       GIT_TAG v1.91.5 SOURCE_SUBDIR _none)
FetchContent_MakeAvailable(minhook imgui)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(HDE ${minhook_SOURCE_DIR}/src/hde/hde64.c)
else()
  set(HDE ${minhook_SOURCE_DIR}/src/hde/hde32.c)
endif()

add_library(GDMenu SHARED
  main.cpp overlay.cpp speedhack.cpp config.cpp
  ${minhook_SOURCE_DIR}/src/buffer.c
  ${minhook_SOURCE_DIR}/src/hook.c
  ${minhook_SOURCE_DIR}/src/trampoline.c
  ${HDE}
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp)

   target_include_directories(GDMenu PRIVATE ${CMAKE_SOURCE_DIR}
  ${minhook_SOURCE_DIR}/include ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_compile_definitions(GDMenu PRIVATE IMGUI_IMPL_WIN32_DISABLE_GAMEPAD NOMINMAX WIN32_LEAN_AND_MEAN)
target_link_libraries(GDMenu PRIVATE opengl32 gdi32 winmm user32 dwmapi)

# La DLL se llama como una que GD ya carga (proxy) => se carga sola, sin Geode
set_target_properties(GDMenu PROPERTIES OUTPUT_NAME "XInput9_1_0" PREFIX "")

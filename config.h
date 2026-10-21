#pragma once
struct Config {
    bool  speedEnabled = false;
    float speed        = 1.0f;
    bool  showFps      = true;
};
extern Config g_cfg;
void configLoad();
void configSave();

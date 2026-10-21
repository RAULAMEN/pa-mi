#pragma once
#include <cstdint>
namespace speedhack {
    bool install();                 // crea los hooks (sin activarlos)
    void setSpeed(double speed);    // 1.0 = normal
    int64_t realQpc();              // contador real, sin escalar
    int64_t realQpcFreq();
}

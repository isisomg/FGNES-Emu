#pragma once

#include <cstdint>

struct Pixel {
    uint8_t r, g, b;
};

// Apenas declare
extern Pixel cores[64];  // NES palette tem 64 cores

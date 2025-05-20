#include "APU.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include "Bus.h"

const uint8_t APU::dutyTable[4][8] = {
    {0,1,0,0,0,0,0,0},
    {0,1,1,0,0,1,1,1},
    {0,1,1,1,1,0,0,0},
    {1,0,0,1,1,1,1,1}
};

const uint8_t APU::triangleTable[32]{
    15, 14, 13, 12, 11, 10,  9,  8,
     7,  6,  5,  4,  3,  2,  1,  0,
     0,  1,  2,  3,  4,  5,  6,  7,
     8,  9, 10, 11, 12, 13, 14, 15
};

void APU::setFrequency(float freq, float can) {
    frequency = freq > 0.0f ? freq : 1.0f;
    canais = can;
    pulse1.timerPeriod = triangle.timerPeriod = noise.timerPeriod = 44100.0f / (frequency * canais);
    dmc.dmcTimerPeriod = 428.0f;
}

void APU::setBus(Bus* b) {
    bus = b;
}

void APU::setEnabled(bool e) {
    enabled = e;
}

void APU::tick(int channel) {
    if (!enabled) return;

    switch (channel) {
    case 1:
        if (!pulse1.enabled) return;
        pulse1.timer -= 1.0f;
        if (pulse1.timer <= 0.0f) {
            pulse1.timer += pulse1.timerPeriod;
            pulse1.phase = (pulse1.phase + 1) % 8;
        }
        break;
    case 2:
        if (!triangle.enabled) return;
        triangle.timer -= 1.0f;
        if (triangle.timer <= 0.0f) {
            triangle.timer += triangle.timerPeriod;
            triangle.phase = (triangle.phase + 1) % 32;
        }
        break;
    case 3:
        if (!noise.enabled) return;
        noise.timer -= 1.0f;
        if (noise.timer <= 0.0f) {
            noise.timer += noise.timerPeriod;
            int bit0 = noise.shiftRegister & 1;
            int bit1or6 = (noise.shiftRegister >> (noise.mode ? 6 : 1)) & 1;
            int feedback = bit0 ^ bit1or6;
            noise.shiftRegister >>= 1;
            noise.shiftRegister |= (feedback << 14);
        }
        break;
    case 4:
        if (!dmc.enabled) return;
        if (dmc.dmcTimer <= 0.0f) {
            dmc.dmcTimer += dmc.dmcTimerPeriod;
            if (dmc.dmcBitCount == 0 && dmc.dmcBytesRemaining > 0) {
                dmc.dmcShiftReg = bus->read(dmc.dmcCurrentAddress++);
                dmc.dmcBitCount = 8;
                dmc.dmcBytesRemaining--;
                if (dmc.dmcCurrentAddress == 0) dmc.dmcCurrentAddress = 0x8000;
                if (dmc.dmcBytesRemaining == 0 && dmc.loop) {
                    dmc.dmcCurrentAddress = dmc.dmcSampleAddress;
                    dmc.dmcBytesRemaining = dmc.dmcSampleLength;
                }
            }
            uint8_t bit = dmc.dmcShiftReg & 1;
            if (bit) {
                if (dmc.dmcOutputLevel <= 125) dmc.dmcOutputLevel += 2;
            }
            else {
                if (dmc.dmcOutputLevel >= 2) dmc.dmcOutputLevel -= 2;
            }
            dmc.dmcShiftReg >>= 1;
            dmc.dmcBitCount--;
        }
        else {
            dmc.dmcTimer -= 1.0f;
        }
        break;
    }
}

float APU::getSample(int channel) const {
    switch (channel) {
    case 1:
        if (!pulse1.enabled || dutyTable[pulse1.duty][pulse1.phase] == 0) return 0.0f;
        return pulse1.volume / 15.0f;
    case 2:
        if (!triangle.enabled) return 0.0f;
        return triangleTable[triangle.phase] / 15.0f;
    case 3:
        if (!noise.enabled || (noise.shiftRegister & 1)) return 0.0f;
        return noise.volume / 15.0f;
    case 4:
        if (!dmc.enabled) return 0.0f;
        return std::min(std::max(dmc.dmcOutputLevel / 127.0f, 0.0f), 1.0f);
    default:
        return 0.0f;
    }
}

void APU::writeRegister(uint16_t addr, uint8_t value) {
    switch (addr) {
    case 0x4000:
        pulse1.duty = (value >> 6) & 0x03;
        pulse1.volume = value & 0x0F;
        break;
    case 0x4002:
        pulse1.timerValue = (pulse1.timerValue & 0xFF00) | value;
        break;
    case 0x4003:
        pulse1.timerValue = (pulse1.timerValue & 0x00FF) | ((value & 0x07) << 8);
        pulse1.phase = 0;
        pulse1.timer = static_cast<float>(pulse1.timerValue);
        break;
    case 0x4010:
        dmc.irqEnabled = value & 0x80;
        dmc.loop = value & 0x40;
        break;
    case 0x4011:
        dmc.dmcOutputLevel = value & 0x7F;
        break;
    case 0x4012:
        dmc.dmcSampleAddress = 0xC000 + (value * 64);
        break;
    case 0x4013:
        dmc.dmcSampleLength = (value * 16) + 1;
        break;
    case 0x4015:
        pulse1.enabled = value & 0x01;
        triangle.enabled = value & 0x04;
        noise.enabled = value & 0x08;
        dmc.enabled = value & 0x10;
        if (!dmc.enabled) dmc.dmcOutputLevel = 0;
        if (dmc.enabled && dmc.dmcBytesRemaining == 0) {
            dmc.dmcCurrentAddress = dmc.dmcSampleAddress;
            dmc.dmcBytesRemaining = dmc.dmcSampleLength;
        }
        break;
    }
}

void APU::step() {
    if (!enabled) return;

    // Atualiza Pulse 1
    if (pulse1.enabled) {
        pulse1.timer -= 1.0f;
        if (pulse1.timer <= 0.0f) {
            pulse1.timer += pulse1.timerPeriod;
            pulse1.phase = (pulse1.phase + 1) % 8;
        }
    }

    // Atualiza Triangle
    if (triangle.enabled) {
        triangle.timer -= 1.0f;
        if (triangle.timer <= 0.0f) {
            triangle.timer += triangle.timerPeriod;
            triangle.phase = (triangle.phase + 1) % 32;
        }
    }

    // Atualiza Noise
    if (noise.enabled) {
        noise.timer -= 1.0f;
        if (noise.timer <= 0.0f) {
            noise.timer += noise.timerPeriod;
            int bit0 = noise.shiftRegister & 1;
            int bit1or6 = (noise.shiftRegister >> (noise.mode ? 6 : 1)) & 1;
            int feedback = bit0 ^ bit1or6;
            noise.shiftRegister >>= 1;
            noise.shiftRegister |= (feedback << 14);
        }
    }

    // Atualiza DMC
    if (dmc.enabled) {
        dmc.dmcTimer -= 1.0f;
        if (dmc.dmcTimer <= 0.0f) {
            dmc.dmcTimer += dmc.dmcTimerPeriod;
            if (dmc.dmcBitCount == 0 && dmc.dmcBytesRemaining > 0) {
                dmc.dmcShiftReg = bus->read(dmc.dmcCurrentAddress++);
                dmc.dmcBitCount = 8;
                dmc.dmcBytesRemaining--;
                if (dmc.dmcCurrentAddress == 0) dmc.dmcCurrentAddress = 0x8000;
                if (dmc.dmcBytesRemaining == 0 && dmc.loop) {
                    dmc.dmcCurrentAddress = dmc.dmcSampleAddress;
                    dmc.dmcBytesRemaining = dmc.dmcSampleLength;
                }
            }
            uint8_t bit = dmc.dmcShiftReg & 1;
            if (bit) {
                if (dmc.dmcOutputLevel <= 125)
                    dmc.dmcOutputLevel += 2;
            }
            else {
                if (dmc.dmcOutputLevel >= 2)
                    dmc.dmcOutputLevel -= 2;
            }
            dmc.dmcShiftReg >>= 1;
            dmc.dmcBitCount--;
        }
    }
}

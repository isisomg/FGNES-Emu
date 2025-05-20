#include "APU.h"
#include <stdio.h>
#include <iostream>

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

void APU::setVolume(int vol) {
    if (vol < 0) vol = 0;
    volume = (vol > 15) ? 15 : vol;
}

void APU::setDuty(int d) {
    if (d >= 0 && d < 4) {
        duty = d;
    }
}

void APU::tick(int channel) {
    if (!enabled) return;

    timer -= 1.0f;

    switch (channel) {
    case 1:
        if (timer <= 0.0f) {
            timer += timerPeriod;
            phase = (phase + 1) % 8;
        }
        break;
    case 2:
        if (timer <= 0.0f) {
            timer += timerPeriod;
            phase = (phase + 1) % 32;
        }
        break;
    case 3:
        if (timer <= 0.0f) {
            timer += timerPeriod;
            int bit0 = shiftRegister & 1;
            int bit1or6 = (shiftRegister >> (mode ? 6 : 1)) & 1;
            int feedback = bit0 ^ bit1or6;
            shiftRegister >>= 1;
            shiftRegister |= (feedback << 14);
        }
        break;
    case 4:
        if (dmcTimer <= 0.0f) {
            dmcTimer += dmcTimerPeriod;
            if (dmcBitCount == 0 && dmcBytesRemaining > 0) {
                dmcShiftReg = bus->read(dmcCurrentAddress++);
                dmcBitCount = 8;
                dmcBytesRemaining--;

                if (dmcCurrentAddress == 0) dmcCurrentAddress = 0x8000;

                if (dmcBytesRemaining == 0 && dmcLoop) {
                    dmcCurrentAddress = dmcSampleAddress;
                    dmcBytesRemaining = dmcSampleLength;
                }
            }

            uint8_t bit = dmcShiftReg & 1;
            if (bit) {
                if (dmcOutputLevel <= 125) dmcOutputLevel += 2;
            }
            else {
                if (dmcOutputLevel >= 2) dmcOutputLevel -= 2;
            }

            dmcShiftReg >>= 1;
            dmcBitCount--;
        }
        else {
            dmcTimer -= 1.0f;
        }
        break;
    default:
        std::cerr << "Canal inválido: " << channel << "\n";
        return;
    }
}

float APU::getSample(int channel) const {
    switch (channel) {
    case 1:
        if (!pulse1_enabled || dutyTable[duty][phase] == 0) return 0.0f;
        return volume / 15.0f;
    case 2:
        if (!triangle_enabled) return 0.0f;
        return triangleTable[phase] / 15.0f;
    case 3:
        if (!noise_enabled || (shiftRegister & 1)) return 0.0f;
        return volume / 15.0f;
    case 4:
        if (!dmc_enabled) return 0.0f;
        return dmcOutputLevel / 127.0f;
    default:
        return 0.0f;
    }
}

void APU::setFrequency(float freq, float can) {
    canais = can;
    frequency = freq;
    if (frequency <= 0.0f) frequency = 1.0f;
    timerPeriod = 44100.0f / (frequency * canais);
}

void APU::setEnabled(bool on) {
    enabled = on;
}

void APU::setBus(Bus* busNovo) {
    this->bus = busNovo;
}

void APU::setMode(bool m) {
    mode = m;
}

void APU::writeRegister(uint16_t addr, uint8_t value) {
    std::cout << "APU Write $" << std::hex << addr << " = " << (int)value << std::endl;
    switch (addr) {
    case 0x4000:
        setDuty((value >> 6) & 0x03);
        setVolume(value & 0x0F);
        break;
    case 0x4002:
        timer = (timer & 0xFF00) | value;
        break;
    case 0x4003:
        timer = (timer & 0x00FF) | ((value & 0x07) << 8);
        phase = 0;
        break;
    case 0x4010:
        dmcIRQEnabled = value & 0x80;
        dmcLoop = value & 0x40;
        break;
    case 0x4011:
        dmcOutputLevel = value & 0x7F;
        break;
    case 0x4012:
        dmcSampleAddress = 0xC000 + (value * 64);
        break;
    case 0x4013:
        dmcSampleLength = (value * 16) + 1;
        break;
    case 0x4015:
        pulse1_enabled = value & 0x01;
        triangle_enabled = value & 0x04;
        noise_enabled = value & 0x08;
        dmc_enabled = value & 0x10;

        if (dmc_enabled && dmcBytesRemaining == 0) {
            dmcCurrentAddress = dmcSampleAddress;
            dmcBytesRemaining = dmcSampleLength;
        }
        break;
    }
}

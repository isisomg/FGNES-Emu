#include "APU.h"
#include "Bus.h"
#include <algorithm>
#include <iostream>

const uint8_t APU::dutyTable[4][8] = {
    {0,1,0,0,0,0,0,0},
    {0,1,1,0,0,1,1,1},
    {0,1,1,1,1,0,0,0},
    {1,0,0,1,1,1,1,1}
};

const uint8_t APU::triangleTable[32] = {
    15,14,13,12,11,10,9,8,
    7,6,5,4,3,2,1,0,
    0,1,2,3,4,5,6,7,
    8,9,10,11,12,13,14,15
};

void APU::setFrequency(float freq, float can) {
    frequency = std::max(freq, 1.0f);
    canais = can;
    float basePeriod = 44100.0f / (frequency * canais);
    pulse1.timerPeriod = triangle.timerPeriod = noise.timerPeriod = basePeriod;
    dmc.dmcTimerPeriod = 428.0f;
}

void APU::setBus(Bus* b) {
    bus = b;
}

void APU::setEnabled(bool e) {
    enabled = e;
}

float APU::getSample(int channel) const {
    switch (channel) {
    case 1:
        return pulse1.getSample();
    case 2:
        return pulse2.getSample();
    case 3:
        return triangle.getSample();
    case 4:
        return noise.getSample();
    case 5:
        return dmc.getSample();
    default:
        return 0.0f;
    }
}


void APU::writeRegister(uint16_t addr, uint8_t value) {
    switch (addr) {
        // Pulse 1
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
        //pulse1.timerPeriod = static_cast<float>(pulse1.timerValue);

        break;

        // Pulse 2
    case 0x4004:
        pulse2.duty = (value >> 6) & 0x03;
        pulse2.volume = value & 0x0F;
        //pulse1.timerPeriod = static_cast<float>(pulse1.timerValue);

        break;
    case 0x4006:
        pulse2.timerValue = (pulse2.timerValue & 0xFF00) | value;
        break;
    case 0x4007:
        pulse2.timerValue = (pulse2.timerValue & 0x00FF) | ((value & 0x07) << 8);
        pulse2.phase = 0;
        pulse2.timer = static_cast<float>(pulse2.timerValue);
        break;

        // Triangle
    case 0x4008:
        triangle.linearCounterReload = value & 0x7F;
        break;

        // Noise
    case 0x400C:
        noise.envelopeLoop = value & 0x20;
        noise.envelopeConstant = value & 0x10;
        noise.envelopeDividerPeriod = value & 0x0F;
        noise.envelopeStart = true;
        break;
    case 0x400E:
        noise.mode = value & 0x80;
        break;
    case 0x400F:
        noise.envelopeStart = true;
        break;

        // DMC
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
        pulse2.enabled = value & 0x02;
        triangle.enabled = value & 0x04;
        noise.enabled = value & 0x08;
        dmc.enabled = value & 0x10;
        if (!dmc.enabled) {
            dmc.dmcOutputLevel = 0;
        }
        else if (dmc.dmcBytesRemaining == 0) {
            dmc.dmcCurrentAddress = dmc.dmcSampleAddress;
            dmc.dmcBytesRemaining = dmc.dmcSampleLength;
        }
        break;
    }
}


void APU::step() {
    if (!enabled) return;

    // Pulse
    if (pulse1.enabled) {
        pulse1.timer -= 1.0f;
        if (pulse1.timer <= 0.0f) {
            pulse1.timer += pulse1.timerPeriod;
            pulse1.phase = (pulse1.phase + 1) % 8;
        }
    }

    if (pulse2.enabled) {
        pulse2.timer -= 1.0f;
        if (pulse2.timer <= 0.0f) {
            pulse2.timer += pulse2.timerPeriod;
            pulse2.phase = (pulse2.phase + 1) % 8;
        }
    }


    // Triangle
    if (triangle.enabled) {
        triangle.timer -= 1.0f;
        if (triangle.timer <= 0.0f) {
            triangle.timer += triangle.timerPeriod;
            triangle.phase = (triangle.phase + 1) % 32;
        }
    }

    // Noise
// Envelope clock (240 Hz)
    if (noise.enabled) {
        noise.envelopeClockTimer += 1.0f;
        if (noise.envelopeClockTimer >= 183.0f) { // 44100 / 240 ? 183.75
            noise.envelopeClockTimer -= 183.0f;

            if (noise.envelopeStart) {
                noise.envelopeStart = false;
                noise.envelopeDecayLevel = 15;
                noise.envelopeDivider = noise.envelopeDividerPeriod;
            }
            else {
                if (noise.envelopeDivider > 0) {
                    noise.envelopeDivider--;
                }
                else {
                    noise.envelopeDivider = noise.envelopeDividerPeriod;
                    if (noise.envelopeDecayLevel > 0) {
                        noise.envelopeDecayLevel--;
                    }
                    else if (noise.envelopeLoop) {
                        noise.envelopeDecayLevel = 15;
                    }
                    // Se loop estiver desligado e já estiver em 0, permanece em 0
                }
            }
        }

        // Shift register do ruído
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

    // DMC
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

            if (dmc.dmcBitCount > 0) {
                if (dmc.dmcShiftReg & 1) {
                    if (dmc.dmcOutputLevel <= 125) dmc.dmcOutputLevel += 2;
                } else {
                    if (dmc.dmcOutputLevel >= 2) dmc.dmcOutputLevel -= 2;
                }
                dmc.dmcShiftReg >>= 1;
                dmc.dmcBitCount--;
            }
        }
    }
}

float APU::getMixedSample() const {
    //float pulseOut = 0.00752f * (pulse1.getSample() + pulse2.getSample());
    float tndOut =
        //0.00851f * triangle.getSample() 
        //+
        0.00494f * noise.getSample();
        //+
        //0.01f * dmc.getSample(); // aumente para testar
    float gain = 20.0f; // ajuste conforme necessário
    return (/*pulseOut +*/ tndOut) * gain;
}
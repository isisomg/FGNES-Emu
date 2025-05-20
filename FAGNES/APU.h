#pragma once
#include <cstdint>
#include <algorithm>

// Declaração antecipada da classe Bus
class Bus;

class APU {
public:
    void setFrequency(float freq, float can);
    void setEnabled(bool e);
    void setBus(Bus* b);
    void step();

    float getSample(int channel) const; // 1 = pulse, 2 = triangle, 3 = noise, 4 = DMC
    void tick(int channel);
    void writeRegister(uint16_t addr, uint8_t value);

private:
    Bus* bus = nullptr; // Declaração corrigida
    bool enabled = true;
    float frequency = 44100.0f;
    float canais = 4.0f;

    struct PulseChannel {
        uint16_t timerValue = 0;
        bool enabled = false;
        int volume = 15;
        int duty = 2;
        int phase = 0;
        float timer = 0;
        float timerPeriod = 0;
    } pulse1;

    struct TriangleChannel {
        bool enabled = false;
        int phase = 0;
        float timer = 0;
        float timerPeriod = 0;
    } triangle;

    struct NoiseChannel {
        bool enabled = false;
        int volume = 15;
        uint16_t shiftRegister = 1;
        bool mode = false;
        float timer = 0;
        float timerPeriod = 0;
    } noise;

    struct DMCChannel {
        bool enabled = false;
        bool loop = false;
        bool irqEnabled = false;
        uint8_t dmcOutputLevel = 0;
        uint8_t dmcShiftReg = 0;
        uint8_t dmcBitCount = 0;
        float dmcTimer = 0;
        float dmcTimerPeriod = 428.0f;
        uint16_t dmcSampleAddress = 0;
        uint16_t dmcCurrentAddress = 0;
        uint16_t dmcSampleLength = 0;
        uint16_t dmcBytesRemaining = 0;
    } dmc;

    static const uint8_t dutyTable[4][8];
    static const uint8_t triangleTable[32];
};

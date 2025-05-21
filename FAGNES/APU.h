#pragma once
#include <cstdint>
#include <algorithm>

class Bus;

class APU {
public:
    void setFrequency(float freq, float can);
    void setEnabled(bool e);
    void setBus(Bus* b);
    void step();
    float getMixedSample() const;  // Novo método para retornar mix de áudio


    float getSample(int channel) const; // 1 = pulse, 2 = triangle, 3 = noise, 4 = DMC
    void writeRegister(uint16_t addr, uint8_t value);

private:
    Bus* bus = nullptr;
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

        float getSample() const {
            if (!enabled) return 0.0f;
            static const int dutyTable[4][8] = {
                {0, 1, 0, 0, 0, 0, 0, 0},
                {0, 1, 1, 0, 0, 0, 0, 0},
                {0, 1, 1, 1, 1, 0, 0, 0},
                {1, 0, 0, 1, 1, 1, 1, 1}
            };
            return dutyTable[duty][phase] ? volume / 15.0f : 0.0f;
        }
    } pulse1;

    PulseChannel pulse2;

    struct TriangleChannel {
        bool enabled = false;
        int phase = 0;
        float timer = 0;
        float timerPeriod = 0;
        int linearCounterReload = 0;

        float getSample() const {
            if (!enabled) return 0.0f;
            static const int triangleTable[32] = {
                15, 14, 13, 12, 11, 10, 9, 8,
                7, 6, 5, 4, 3, 2, 1, 0,
                0, 1, 2, 3, 4, 5, 6, 7,
                8, 9, 10, 11, 12, 13, 14, 15
            };
            return triangleTable[phase] / 15.0f;
        }
    } triangle;

    struct NoiseChannel {
        bool enabled = false;
        int volume = 15;
        uint16_t shiftRegister = 1;
        bool mode = false;
        float timer = 0;
        float timerPeriod = 0;
        bool envelopeStart = false;
        bool envelopeLoop = false;
        bool envelopeConstant = false;
        uint8_t envelopeDecayLevel = 15;
        uint8_t envelopeDivider = 0;
        uint8_t envelopeDividerPeriod = 0;

        float getSample() const {
            if (!enabled) return 0.0f;
            return (shiftRegister & 1) ? 0.0f : volume / 15.0f;
        }
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
        
        float getSample() const {
            return dmcOutputLevel / 127.0f;
        }
    } dmc;

    static const uint8_t dutyTable[4][8];
    static const uint8_t triangleTable[32];
};

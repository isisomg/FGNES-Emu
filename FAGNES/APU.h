#pragma once
#include <cstdint>
#include <algorithm>

class Bus;

class APU {
public:
    float PITCH_ADJUST = 1.05f;
    uint8_t readRegister(uint16_t addr);
    void clockFrameSequencer();
    void stepCpuCycles(float cpuCycles);
    //void setFrequency(float freq, float can);
    void setEnabled(bool e);
    void setBus(Bus* b);
    void step();
    //void fetchDMCByte();
    float getMixedSample() const;  // retornar mix de �udio


    float getSample(int channel) const; // 1 = pulse, 2 = triangle, 3 = noise, 4 = DMC
    void writeRegister(uint16_t addr, uint8_t value);

private:
    float cpuCycleRemainder = 0.0f;
    int frameSequencerStep = 0;
    Bus* bus = nullptr;
    bool enabled = true;
    float frequency = 44100.0f;
    float canais = 4.0f;

    struct PulseChannel {
        uint16_t timerValue = 0;
        bool enabled = false;
        int volume = 15;
        int dutyCycle = 0;
        int phase = 0;
        float timer = 0;
        float timerPeriod = 0;
        bool isInverted = false; // Canal 2 invertido para testes de cancelamento

        // Campos necess�rios
        bool envelopeStart = false;
        int envelopeDivider = 0;
        int envelopeDecayLevel = 0;
        uint8_t envelopeDividerPeriod = 0;
        bool envelopeLoop = false;
        bool envelopeConstant = false;

        // sweep
        bool sweepEnable = false;
        uint8_t sweepPeriod = 0;
        bool sweepNegate = false;
        uint8_t sweepShift = 0;
        int sweepDivider = 0;
        bool sweepReload = false;
        bool sweepMute = false;

        int lengthCounter = 0;
        bool lengthCounterHalt = false;

        // Sweep
        void clockSweep() {
            sweepMute = (timerValue < 8);

            if (sweepDivider == 0 && sweepEnable && !sweepMute) {
                // calcula nova period
                int change = timerValue >> sweepShift;
                if (sweepNegate) change = -change - (isInverted ? 1 : 0);
                uint16_t newPeriod = timerValue + change;
                // se overflow, silencia
                if (newPeriod > 0x7FF) {
                    sweepMute = true;
                }
                else {
                    timerValue = newPeriod;
                    timerPeriod = (timerValue + 1) * 2;
                    sweepMute = false;
                }
            }
            if (sweepDivider == 0 || sweepReload) {
                sweepDivider = sweepPeriod;
                sweepReload = false;
            }
            else {
                --sweepDivider;
            }
        }


        // Envelope
        void clockEnvelope() {
            if (envelopeStart) {
                envelopeStart = false;
                envelopeDecayLevel = 15;
                envelopeDivider = envelopeDividerPeriod;
            }
            else {
                if (--envelopeDivider < 0) {
                    envelopeDivider = envelopeDividerPeriod;
                    if (envelopeDecayLevel > 0) {
                        --envelopeDecayLevel;
                    }
                    else if (envelopeLoop) {
                        envelopeDecayLevel = 15;
                    }
                }
            }
        }

        // Length counter
        void clockLength() {
            if (!lengthCounterHalt && lengthCounter > 0) {
                --lengthCounter;
            }
        }

        float getSample() const {
            if (!enabled || timerValue < 8 || lengthCounter == 0 || sweepMute) return 0.0f;

            // usa envelope ou volume constante
            uint8_t duty = APU::dutyTable[dutyCycle][phase];
            float envelopeVolume = envelopeConstant ? envelopeDividerPeriod : envelopeDecayLevel;
            return duty ? (envelopeVolume / 15.0f) : 0.0f;
        }
    } pulse1;

    PulseChannel pulse2;

    struct TriangleChannel {
        bool enabled = false;
        int phase = 0;
        uint16_t timerValue = 0;
        float timer = 0;
        float timerPeriod = 0;
        int linearCounterReload = 0;
        uint8_t sequencerStep = 0;

        // Campos necess�rios
        int linearCounter = 0;
        bool linearReloadFlag = false;
        bool linearControlFlag = false;

        int lengthCounter = 0;
        bool lengthCounterHalt = false;

        // Linear counter
        void clockLinearCounter() {
            if (linearReloadFlag) {
                linearCounter = linearCounterReload;
            }
            else if (linearCounter > 0) {
                --linearCounter;
            }
            if (!linearControlFlag) {
                linearReloadFlag = false;
            }
        }

        // Length counter
        void clockLength() {
            if (!lengthCounterHalt && lengthCounter > 0) {
                --lengthCounter;
            }
        }

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
        bool lengthCounterHalt = false;
        uint8_t lengthCounter = 0;

        float envelopeClockTimer = 0.0f; // para clockar o envelope a 240Hz
        bool enabled = false;
        int volume = 15;
        uint16_t shiftRegister = 1;
        bool mode = false;
        int phase = 0;
        float timer = 0;
        float timerPeriod = 0;

        // Campos necess�rios
        bool envelopeStart = false;
        int envelopeDivider = 0;
        int envelopeDecayLevel = 0;
        uint8_t envelopeDividerPeriod = 0;
        bool envelopeLoop = false;
        bool envelopeConstant = false;

        // Envelope
        void clockEnvelope() {
            if (envelopeStart) {
                envelopeStart = false;
                envelopeDecayLevel = 15;
                envelopeDivider = envelopeDividerPeriod;
            }
            else {
                if (--envelopeDivider < 0) {
                    envelopeDivider = envelopeDividerPeriod;
                    if (envelopeDecayLevel > 0) {
                        --envelopeDecayLevel;
                    }
                    else if (envelopeLoop) {
                        envelopeDecayLevel = 15;
                    }
                }
            }
        }

        // Length counter
        void clockLength() {
            if (!lengthCounterHalt && lengthCounter > 0) {
                --lengthCounter;
            }
        }

        float getSample() const {
            if (!enabled || lengthCounter == 0 || (shiftRegister & 1)) return 0.0f;

            int volume = envelopeConstant ? envelopeDividerPeriod : envelopeDecayLevel;
            return volume / 15.0f;
        }
    } noise;

    struct DMCChannel {
        bool enabled = false;

        bool irqEnabled = false;
        bool irqFlag = false;
        bool loop = false;

        uint16_t dmcSampleAddress = 0;
        uint16_t dmcCurrentAddress = 0;
        uint16_t dmcSampleLength = 0;
        uint16_t dmcBytesRemaining = 0;

        uint8_t dmcOutputLevel = 0; // 7-bit unsigned
        uint8_t dmcShiftReg = 0;
        uint8_t dmcBitCount = 0;

        uint16_t dmcTimer = 0;
        float dmcTimerPeriod = 428; // default period

        bool bufferEmpty = true;
        uint8_t sampleBuffer = 0;

        bool silence = true;

        Bus* bus = nullptr; // refer�ncia ao barramento

        float getSample() const {
            return dmcOutputLevel / 127.0f;
        }
    } dmc;

    static const uint8_t dutyTable[4][8];
    static const uint8_t triangleTable[32];
};
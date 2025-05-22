#pragma once
#include <cstdint>
#include <algorithm>

class Bus;

class APU {
public:
    void clockFrameSequencer();
    void stepCpuCycles(float cpuCycles);
    //void setFrequency(float freq, float can);
    void setEnabled(bool e);
    void setBus(Bus* b);
    void step();
    float getMixedSample() const;  // retornar mix de áudio


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
        int duty = 2;
        int phase = 0;
        float timer = 0;
        float timerPeriod = 0;
        bool isInverted = false; // Canal 2 invertido para testes de cancelamento

        // Campos necessários
        bool envelopeStart = false;
        int envelopeDivider = 0;
        int envelopeDecayLevel = 0;
        uint8_t envelopeDividerPeriod = 0;
        bool envelopeLoop = false;
        bool envelopeConstant = false;

        int lengthCounter = 0;
        bool lengthCounterHalt = false;

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
            if (!enabled || timerValue < 8) return 0.0f;

            float amp = APU::dutyTable[duty][phase] ? volume : 0.0f;

            // Canal 2 invertido para testes de cancelamento
            return isInverted ? -amp : amp;
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

        // Campos necessários
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

        // Registradores
        void writeControl(uint8_t value) {
            linearControlFlag = (value >> 7) & 1;
            linearCounterReload = value & 0x7F;
            lengthCounterHalt = linearControlFlag;
        }

        void writeTimerLow(uint8_t value) {
            timerValue = (timerValue & 0x0700) | value;
        }

        void writeTimerHigh(uint8_t value) {
            timerValue = (timerValue & 0x00FF) | ((value & 0x07) << 8);
            lengthCounter = triangleTable[(value >> 3) & 0x1F];
            linearReloadFlag = true;
            timerPeriod = timerValue + 1;
            timer = timerPeriod;
            sequencerStep = 0;
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
        float timer = 0;
        float timerPeriod = 0;

        // Campos necessários
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
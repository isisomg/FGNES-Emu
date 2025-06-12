#pragma once
#include <cstdint>
#include <algorithm>
#include <iostream>

class Bus;

class APU {
public:
    void reset();
    bool irq() {
        bool status = irqFlag;
        irqFlag = false;
        return status;
    }
    float PITCH_ADJUST = 1.0f;
    uint8_t readRegister(uint16_t addr);
    void clockFrameSequencer();
    void stepCpuCycles(float cpuCycles);
    //void setFrequency(float freq, float can);
    void setEnabled(bool e);
    void setBus(Bus* b);
    void step();
    APU();
    //void fetchDMCByte();
    float getMixedSample() const;  // retornar mix de áudio


    float getSample(int channel) const; // 1 = pulse, 2 = triangle, 3 = noise, 4 = DMC
    void writeRegister(uint16_t addr, uint8_t value);

private:
    float cpuCycleRemainder = 0.0f;
    int frameSequencerStep = 0;
    int frameSequencerCounter = 0;
    Bus* bus = nullptr;
    bool enabled = true;
    float frequency = 44100.0f;
    float canais = 4.0f;
    uint8_t frameCounterRegister = 0; // Registrador $4017
    int frameSequencerMode = 0;
    uint64_t totalCycles = 0;
    bool inhibitIrq = false;
    bool irqFlag = false;
    void clockEnvelopesAndLinear();
    void clockLengthAndSweep();

    struct PulseChannel {
        bool enabled = false;
        uint16_t timerValue = 0;    // Período de 11 bits raw
        int timer = 0;              // Contador de ciclo para o período da onda
        int dutyCycle = 0;          // 0-3, para selecionar a forma da onda
        int phase = 0;              // 0-7, passo atual na forma da onda
        bool isInverted = false;    // Usado para o sweep do canal 2

        // Envelope
        bool envelopeStart = false;
        int envelopeDivider = 0;
        int envelopeDecayLevel = 0;
        uint8_t envelopeDividerPeriod = 0;
        bool envelopeLoop = false;
        bool envelopeConstant = false;

        // Sweep
        bool sweepEnable = false;
        uint8_t sweepPeriod = 0;
        bool sweepNegate = false;
        uint8_t sweepShift = 0;
        int sweepDivider = 0;
        bool sweepReload = false;

        // Length Counter
        int lengthCounter = 0;
        bool lengthCounterHalt = false;

        // Método para avançar o envelope (chamado a ~240Hz)
        void clockEnvelope() {
            if (envelopeStart) {
                envelopeStart = false;
                envelopeDecayLevel = 15;
                envelopeDivider = envelopeDividerPeriod + 1;
            }
            else {
                if (--envelopeDivider <= 0) {
                    envelopeDivider = envelopeDividerPeriod + 1;
                    if (envelopeDecayLevel > 0) {
                        envelopeDecayLevel--;
                    }
                    else if (envelopeLoop) {
                        envelopeDecayLevel = 15;
                    }
                }
            }
        }

        // Método para avançar a unidade de sweep (chamado a ~120Hz)
        void clockSweep() {
            uint16_t targetPeriod = 0;
            bool muting = false;

            // Calcula o período alvo
            if (sweepEnable && sweepShift > 0) {
                uint16_t change = timerValue >> sweepShift;
                if (sweepNegate) {
                    targetPeriod = timerValue - change;
                    if (!isInverted) {
                        targetPeriod--;
                    }
                }
                else {
                    targetPeriod = timerValue + change;
                }
            }

            muting = (timerValue < 8 || targetPeriod > 0x7FF);

            if (sweepDivider > 0) {
                sweepDivider--;
            }
            else {
                sweepDivider = sweepPeriod;
                if (sweepEnable && sweepShift > 0 && !muting) {
                    timerValue = targetPeriod;
                }
            }
            if (sweepReload) {
                sweepDivider = sweepPeriod;
                sweepReload = false;
            }
        }

        // Método para avançar o contador de duração (chamado a ~120Hz)
        void clockLength() {
            if (!lengthCounterHalt && lengthCounter > 0) {
                lengthCounter--;
            }
        }

        bool isSweepMuting() const {
            if (timerValue < 8 || timerValue > 0x7FF) {
                return true;
            }

            if (sweepEnable && !sweepNegate) {
                uint16_t change = timerValue >> sweepShift;
                if ((timerValue + change) > 0x7FF) {
                    return true;
                }
            }

            return false;
        }

        float getSample() const {
            if (!enabled || lengthCounter == 0 || isSweepMuting() || APU::dutyTable[dutyCycle][phase] == 0) {
                return 0.0f;
            }

            float volume = envelopeConstant ? (float)envelopeDividerPeriod : (float)envelopeDecayLevel;
            return volume / 15.0f;
        }
    }pulse1, pulse2;

    struct TriangleChannel {
        bool enabled = false;
        int phase = 0;
        uint16_t timerValue = 0; // Período de 11 bits raw
        float timer = 0;         // Contador de ciclo da CPU para o período
        float timerPeriod = 0;   // Valor do timer a ser recarregado

        int linearCounter = 0;
        int linearCounterReload = 0;
        bool linearReloadFlag = false;
        bool linearControlFlag = false;

        int lengthCounter = 0;

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

        void clockLength() {
            if (!linearControlFlag && lengthCounter > 0) {
                --lengthCounter;
            }
        }

        float getSample() const {
            if (!enabled || linearCounter == 0 || lengthCounter == 0 || timerValue < 2) {
                return 0.0f;
            }
            return APU::triangleTable[phase] / 15.0f;
        }
    }triangle;

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

        Bus* bus = nullptr; // referência ao barramento

        float getSample() const {
            return dmcOutputLevel / 127.0f;
        }
    } dmc;

    static const uint8_t dutyTable[4][8];
    static const uint8_t triangleTable[32];
};
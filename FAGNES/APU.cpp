#include "APU.h"
#include "Bus.h"
#include <algorithm>
#include <iostream>

// Tabela de períodos do canal de ruído (Noise)
const uint16_t noisePeriodTable[16] = {
    4, 8, 16, 32, 64, 96, 128, 160,
    202, 254, 380, 508, 762, 1016, 2034, 4068
};

// Tabela de contadores de duração (Length Counter)
const uint8_t lengthTable[32] = {
    0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
    0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
    0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
    0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
};

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

static const uint16_t dmcPeriodTable[16] = {
  428,380,340,320,286,254,226,214,
  190,160,142,128,106, 85, 72, 54
};

//void APU::setFrequency(float freq, float can) {
//    frequency = std::max(freq, 1.0f);
//    canais = can;
//    float basePeriod = 44100.0f / (frequency * canais);
//
//    pulse1.timerPeriod = triangle.timerPeriod = basePeriod;
//    dmc.dmcTimerPeriod = 428.0f;
//}

void APU::setBus(Bus* b) {
    bus = b;
}

void APU::setEnabled(bool e) {
    enabled = e;
}

float APU::getSample(int channel) const {
    switch (channel) {
    case 1: return pulse1.getSample();
    case 2: return pulse2.getSample();
    case 3: return triangle.getSample();
    case 4: return noise.getSample();
    case 5: return dmc.getSample();
    default: return 0.0f;
    }
}

void APU::writeRegister(uint16_t addr, uint8_t value) {
    switch (addr) {
        // Pulse 1
    case 0x4000:
        pulse1.envelopeConstant = (value & 0x10) != 0;
        pulse1.envelopeDividerPeriod = value & 0x0F;
        pulse1.envelopeLoop = (value & 0x20) != 0;
        pulse1.lengthCounterHalt = (value & 0x20) != 0;
        pulse1.envelopeStart = true;
        break;
    case 0x4001: { // Pulse 1 sweep
        pulse1.sweepEnable = (value & 0x80) != 0;
        pulse1.sweepPeriod = ((value >> 4) & 0x07) + 1;
        pulse1.sweepNegate = (value & 0x08) != 0;
        pulse1.sweepShift = (value & 0x07);
        pulse1.sweepReload = true;
        break;
    }
    case 0x4002:
        pulse1.timerValue = (pulse1.timerValue & 0xFF00) | value;
        break;
    case 0x4003:
        pulse1.timerValue = (pulse1.timerValue & 0x00FF) | ((value & 0x07) << 8);
        pulse1.phase = 0;
        pulse1.envelopeStart = true;
        pulse1.timerPeriod = (float)(((pulse1.timerValue + 1) * 2) * PITCH_ADJUST);
        pulse1.timer = pulse1.timerPeriod;
        pulse1.lengthCounter = lengthTable[(value >> 3) & 0x1F];
        pulse1.sweepDivider = pulse1.sweepPeriod;
        break;

        // Pulse 2
    case 0x4004:
        pulse2.envelopeConstant = (value & 0x10) != 0;
        pulse2.envelopeDividerPeriod = value & 0x0F;
        pulse2.envelopeLoop = (value & 0x20) != 0;
        pulse2.lengthCounterHalt = (value & 0x20) != 0;
        pulse2.envelopeStart = true;
        break;
    case 0x4005: { // Pulse 1 sweep
        pulse2.sweepEnable = (value & 0x80) != 0;
        pulse2.sweepPeriod = ((value >> 4) & 0x07) + 1;
        pulse2.sweepNegate = (value & 0x08) != 0;
        pulse2.sweepShift = (value & 0x07);
        pulse2.sweepReload = true;
        break;
    }

    case 0x4006:
        pulse2.timerValue = (pulse2.timerValue & 0xFF00) | value;
        break;
    case 0x4007:
        pulse2.timerValue = (pulse2.timerValue & 0x00FF) | ((value & 0x07) << 8);
        pulse2.phase = 0;
        pulse2.envelopeStart = true;
        //pulse2.timerPeriod = (pulse2.timerValue + 1) * 2;
        pulse2.timerPeriod = (float)(((pulse2.timerValue + 1) * 2) * PITCH_ADJUST);
        pulse2.timer = pulse2.timerPeriod;
        pulse2.lengthCounter = lengthTable[(value >> 3) & 0x1F];
        pulse2.sweepDivider = pulse2.sweepPeriod;
        break;

        // Triangle
    case 0x4008:
        triangle.linearControlFlag = (value >> 7) & 1;
        triangle.linearCounterReload = value & 0x7F;
        triangle.lengthCounterHalt = triangle.linearControlFlag;
        break;
    case 0x400A:
        triangle.timerValue = (triangle.timerValue & 0x0700) | value;
        break;
    case 0x400B:
        triangle.timerValue = (triangle.timerValue & 0x00FF) | ((value & 0x07) << 8);
        triangle.lengthCounter = triangleTable[(value >> 3) & 0x1F];
        triangle.linearReloadFlag = true;
        //timerPeriod = timerValue + 1;
        triangle.timerPeriod = (float)((triangle.timerValue + 1)* PITCH_ADJUST);
        triangle.timer = triangle.timerPeriod;
        triangle.sequencerStep = 0;
        break;

        // Noise
    case 0x400C:
        noise.envelopeLoop = noise.lengthCounterHalt = (value & 0x20) != 0;
        noise.envelopeConstant = (value & 0x10) != 0;
        noise.envelopeDividerPeriod = value & 0x0F;
        noise.envelopeStart = true;
        break;
    case 0x400E: {
        noise.mode = value & 0x80;
        uint8_t index = value & 0x0F;
        const float cpuCyclesPerSample = 1789773.0f / 44100.0f;
        noise.timerPeriod = (float)(noisePeriodTable[index] * PITCH_ADJUST);
        noise.timer = noise.timerPeriod;
        break;
    }
    case 0x400F:
        noise.envelopeStart = true;
        noise.timer = noise.timerPeriod;
        if (noise.enabled) {
            uint8_t index = (value >> 3) & 0x1F;
            noise.lengthCounter = lengthTable[index];
        }
        break;

        // DMC
    case 0x4010:
        dmc.irqEnabled = value & 0x80;
        dmc.loop = value & 0x40;
        dmc.dmcTimerPeriod = (float)(dmcPeriodTable[value & 0x0F] * PITCH_ADJUST);
        dmc.dmcTimer = dmc.dmcTimerPeriod;
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
            // **Zera tudo** para parar imediatamente
            dmc.dmcOutputLevel = 0;
            dmc.dmcBytesRemaining = 0;
            dmc.dmcBitCount = 0;
            dmc.dmcShiftReg = 0;
            dmc.bufferEmpty = true;
            dmc.silence = true;
            dmc.irqFlag = false;
        }
        else if (dmc.dmcBytesRemaining == 0) {
            // **(re)inicia** o sample
            dmc.dmcCurrentAddress = dmc.dmcSampleAddress;
            dmc.dmcBytesRemaining = dmc.dmcSampleLength;
            // garante estado limpo
            dmc.dmcBitCount = 0;
            dmc.bufferEmpty = true;
            dmc.silence = true;
            dmc.irqFlag = false;
        }
        break;
    }
}

uint8_t APU::readRegister(uint16_t addr) {
    if (addr == 0x4015) {
        uint8_t status = 0;
        if (pulse1.enabled)   status |= 0x01;
        if (pulse2.enabled)   status |= 0x02;
        if (triangle.enabled) status |= 0x04;
        if (noise.enabled)    status |= 0x08;
        if (dmc.enabled)      status |= 0x10;
        if (dmc.irqFlag)      status |= 0x80;
        dmc.irqFlag = false;    // limpar IRQ ao ler
        return status;
    }
    return 0;
}


void APU::step() {
    if (!enabled) return;

    // Pulse 1
    if (pulse1.enabled) {
        if ((pulse1.timer -= 1.0f) <= 0.0f) {
            pulse1.timer += pulse1.timerPeriod;
            pulse1.phase = (pulse1.phase + 1) % 8;
        }
    }

    // Pulse 2
    if (pulse2.enabled) {
        if ((pulse2.timer -= 1.0f) <= 0.0f) {
            pulse2.timer += pulse2.timerPeriod;
            pulse2.phase = (pulse2.phase + 1) % 8;
        }
    }

    // Triangle
    if (triangle.enabled) {
        if ((triangle.timer -= 1.0f) <= 0.0f) {
            triangle.timer += triangle.timerPeriod;
            triangle.phase = (triangle.phase + 1) % 32;
        }
    }

    // Noise
    if (noise.enabled) {
        ////noise.shiftRegister = 1;  // LFSR precisa estar com pelo menos 1 bit ativo
        //// Length Counter
        //if (!noise.lengthCounterHalt && noise.lengthCounter > 0) {
        //    noise.lengthCounter--;
        //}

        //// Envelope
        //noise.envelopeClockTimer += 1.0f;
        //if (noise.envelopeClockTimer >= 183.0f) {
        //    noise.envelopeClockTimer -= 183.0f;

        //    if (noise.envelopeStart) {
        //        noise.envelopeStart = false;
        //        noise.envelopeDecayLevel = 15;
        //        noise.envelopeDivider = noise.envelopeDividerPeriod;
        //    }
        //    else {
        //        if (noise.envelopeDivider > 0) {
        //            noise.envelopeDivider--;
        //        }
        //        else {
        //            noise.envelopeDivider = noise.envelopeDividerPeriod;
        //            if (noise.envelopeDecayLevel > 0) {
        //                noise.envelopeDecayLevel--;
        //            }
        //            else if (noise.envelopeLoop) {
        //                noise.envelopeDecayLevel = 15;
        //            }
        //        }
        //    }
        //}

        // Shift Register
        if (--noise.timer <= 0.0f) {
            noise.timer += noise.timerPeriod;

            uint16_t feedback;
            if (noise.mode) {
                // Modo curto: XOR entre bit 0 e bit 6
                feedback = (noise.shiftRegister & 0x01) ^ ((noise.shiftRegister >> 6) & 0x01);
            }
            else {
                // Modo longo: XOR entre bit 0 e bit 1
                feedback = (noise.shiftRegister & 0x01) ^ ((noise.shiftRegister >> 1) & 0x01);
            }

            noise.shiftRegister >>= 1;
            noise.shiftRegister |= (feedback << 14);
            noise.phase = noise.shiftRegister & 1;
        }

        // DMC
        // DMC
        if (dmc.enabled) {
            if ((dmc.dmcTimer -= 1.0f) <= 0.0f) {
                dmc.dmcTimer += dmc.dmcTimerPeriod;

                // 1) MEMORY READER: preenche sampleBuffer
                if (dmc.bufferEmpty && dmc.dmcBytesRemaining > 0) {
                    dmc.sampleBuffer = bus->read(dmc.dmcCurrentAddress++);
                    dmc.bufferEmpty = false;
                    dmc.dmcBytesRemaining--;
                    if (dmc.dmcCurrentAddress > 0xFFFF) dmc.dmcCurrentAddress = 0x8000;

                    if (dmc.dmcBytesRemaining == 0) {
                        if (dmc.loop) {
                            dmc.dmcCurrentAddress = dmc.dmcSampleAddress;
                            dmc.dmcBytesRemaining = dmc.dmcSampleLength;
                        }
                        else if (dmc.irqEnabled) {
                            dmc.irqFlag = true;
                        }
                    }
                }

                // 2) OUTPUT CYCLE: quando bitCount == 0, carrega o shiftReg
                if (dmc.dmcBitCount == 0) {
                    if (dmc.bufferEmpty) {
                        dmc.silence = true;
                    }
                    else {
                        dmc.silence = false;
                        dmc.dmcShiftReg = dmc.sampleBuffer;
                        dmc.bufferEmpty = true;
                    }
                    dmc.dmcBitCount = 8;
                }

                // 3) SHIFT & DELTA: apenas se não for silêncio
                if (!dmc.silence) {
                    if (dmc.dmcShiftReg & 1) {
                        if (dmc.dmcOutputLevel <= 125) dmc.dmcOutputLevel += 2;
                    }
                    else {
                        if (dmc.dmcOutputLevel >= 2) dmc.dmcOutputLevel -= 2;
                    }
                }
                dmc.dmcShiftReg >>= 1;
                dmc.dmcBitCount--;
            }
        }
    }
}

float APU::getMixedSample() const {
    float pulseOut = 0.00752f * (pulse1.getSample() + pulse2.getSample());
    float tndOut =
        0.00851f * triangle.getSample() +
        0.00494f * noise.getSample() +
        0.01f * dmc.getSample();

    float gain = 20.0f; // ajuste de volume geral
    return (pulseOut + tndOut) * gain;
}

//float APU::getMixedSample() const {
//    int p1 = pulse1.getSample();   // já é 0–15
//    int p2 = pulse2.getSample();
//    float pulseOut = 0.0f;
//    int psum = p1 + p2;
//    if (psum > 0) {
//        pulseOut = 95.88f / (8128.0f / psum + 100.0f);
//    }
//
//    float t = triangle.getSample();
//    float n = noise.getSample();
//    float d = dmc.getSample();
//    // fórmula oficial para TND
//    float tndOut = 159.79f / (1.0f/(t/8227.0f + n/12241.0f + d/22638.0f) + 100.0f);
//
//    return pulseOut + tndOut;
//}

void APU::stepCpuCycles(float cpuCycles) {
    cpuCycleRemainder += cpuCycles;
    // chama step() para cada ciclo inteiro de CPU
    while (cpuCycleRemainder >= 1.0f) {
        step();
        cpuCycleRemainder -= 1.0f;
        // a cada 7457 ciclos, clock do frame sequencer (~240Hz)
        static const int SEQ_PERIOD = 7457;
        static int seqCounter = 0;
        if (++seqCounter >= SEQ_PERIOD) {
            seqCounter = 0;
            clockFrameSequencer();
        }
    }
}

void APU::clockFrameSequencer() {
    frameSequencerStep = (frameSequencerStep + 1) & 3;
    // passo 1 e 3: clock envelopes e linear counter
    if (frameSequencerStep == 1 || frameSequencerStep == 3) {
        pulse1.clockEnvelope();  pulse2.clockEnvelope();
        noise.clockEnvelope();   triangle.clockLinearCounter();
    }
    // passo 0 e 2: clock length counters (e sweep)
    if (frameSequencerStep == 0 || frameSequencerStep == 2) {
        pulse1.clockLength();  pulse2.clockLength();  noise.clockLength();
        if (frameSequencerStep == 2) {
            triangle.clockLength();
            pulse1.clockSweep();
            pulse2.clockSweep();
        }
    }
}
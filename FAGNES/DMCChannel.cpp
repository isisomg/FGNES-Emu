#include "DMCChannel.h"

// Tabela de períodos (em número de ciclos da CPU) para cada índice de taxa
static const float DMC_RATE_TABLE[16] = {
    428, 380, 340, 320, 286, 254, 226, 214,
    190, 160, 142, 128, 106, 85,  72,  54
};

// so pra testar o canal de audio
const uint8_t fakeSample[] = {
    64, 80, 96, 112, 127, 112, 96, 80,
    64, 48, 32, 16, 0, 16, 32, 48
};


DMCChannel::DMCChannel()
    : currentAddress(0), bytesRemaining(0),
    sampleBuffer(0), sampleBufferEmpty(true),
    shiftRegister(0), bitCounter(8),
    deltaCounter(64), // Valor inicial padrão do contador delta
    timer(0), timerPeriod(DMC_RATE_TABLE[0]),
    enabled(false), loopFlag(false), irqEnabled(false),
    sampleStartAddress(0), sampleLength(0)
{
}

void DMCChannel::setEnabled(bool value) {
    enabled = value;
    if (!value) {
        bytesRemaining = 0;
    }
    else if (bytesRemaining == 0) {
        restartSample();
    }
}

void DMCChannel::setRate(uint8_t index) {
    if (index < 16) {
        timerPeriod = DMC_RATE_TABLE[index];
    }
}

void DMCChannel::setSampleAddress(uint8_t address) {
    sampleStartAddress = 0xC000 + (address * 64);
}

void DMCChannel::setSampleLength(uint8_t length) {
    sampleLength = (length * 16) + 1;
}

void DMCChannel::setLoop(bool loop) {
    loopFlag = loop;
}

void DMCChannel::setIRQEnabled(bool irq) {
    irqEnabled = irq;
}

void DMCChannel::restartSample() {
    currentAddress = sampleStartAddress;
    bytesRemaining = sampleLength;
}

void DMCChannel::setSampleData(const uint8_t* data, size_t size) { // simula a memória do sample pro funcionamento do tick, mais tarde vai dar pra tirar provavelmente, ou adaptar
    sampleData = data;
    sampleDataSize = size;
}

void DMCChannel::tick() {
    if (!enabled) return;

    timer -= 1.0f;
    if (timer <= 0.0f) {
        timer += timerPeriod;

        if (!sampleBufferEmpty) {
            if ((shiftRegister & 1) == 1) {
                if (deltaCounter <= 125) deltaCounter += 2;
            }
            else {
                if (deltaCounter >= 2) deltaCounter -= 2;
            }
            shiftRegister >>= 1;
            bitCounter--;

            if (bitCounter == 0) {
                if (!sampleBufferEmpty) {
                    shiftRegister = sampleBuffer;
                    bitCounter = 8;
                    sampleBufferEmpty = true;
                }
            }
        }

        if (sampleBufferEmpty && bytesRemaining > 0) {
            // Leitura da "memória"
            if (currentAddress < sampleDataSize) {
                sampleBuffer = sampleData[currentAddress];
                sampleBufferEmpty = false;
                currentAddress++;
                bytesRemaining--;
                if (bytesRemaining == 0 && loopFlag) {
                    restartSample();
                }
            }
        }
    }
}

float DMCChannel::getSample() const {
    return enabled ? (deltaCounter / 127.0f) : 0.0f;
}

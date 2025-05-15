#include "DMCChannel.h"
#include <stdio.h>
#include <iostream>

// Tabela de per�odos (em n�mero de ciclos da CPU) para cada �ndice de taxa
static const float DMC_RATE_TABLE[16] = {
    428, 380, 340, 320, 286, 254, 226, 214,
    190, 160, 142, 128, 106, 85,  72,  54
};

// so pra testar o canal de audio
const uint8_t DMCChannel::fakeSample[32] = {
    64, 80, 96, 112, 127, 112, 96, 80,
    64, 48, 32, 16, 0, 16, 32, 48,
    64, 80, 96, 112, 127, 112, 96, 80,
    64, 48, 32, 16, 0, 16, 32, 48
};

//const uint8_t DMCChannel::fakeSample[1] = { 0xAA };  // padr�o bin�rio 10101010


DMCChannel::DMCChannel()
    : currentAddress(0), bytesRemaining(0),
    sampleBuffer(0), sampleBufferEmpty(true),
    shiftRegister(0), bitCounter(8),
    deltaCounter(64), // Valor inicial padr�o do contador delta
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

void DMCChannel::setSampleData(const uint8_t* data, size_t size) { // simula a mem�ria do sample pro funcionamento do tick, mais tarde vai dar pra tirar provavelmente, ou adaptar
    sampleData = data;
    sampleDataSize = size;
    std::cout << "restartSample() chamado! Endere�o: " << sampleStartAddress
        << ", comprimento: " << sampleLength << "\n";
}

//void DMCChannel::tick() {
//    if (!enabled) return;
//
//    timer -= 1.0f;
//    if (timer <= 0.0f) {
//        timer += timerPeriod;
//
//        printf("tick: %d\n", deltaCounter);
//
//        if (!sampleBufferEmpty) {
//            if ((shiftRegister & 1) == 1) {
//                if (deltaCounter <= 125) deltaCounter += 2;
//            }
//            else {
//                if (deltaCounter >= 2) deltaCounter -= 2;
//            }
//            shiftRegister >>= 1;
//            bitCounter--;
//
//            if (bitCounter == 0) {
//                if (!sampleBufferEmpty) {
//                    shiftRegister = sampleBuffer;
//                    bitCounter = 8;
//                    sampleBufferEmpty = true;
//                }
//            }
//        }
//
//        if (sampleBufferEmpty && bytesRemaining > 0) {
//            // Leitura da "mem�ria"
//            if (currentAddress < sampleDataSize) {
//                sampleBuffer = sampleData[currentAddress];
//                sampleBufferEmpty = false;
//                currentAddress++;
//                bytesRemaining--;
//                if (bytesRemaining == 0 && loopFlag) {
//                    restartSample();
//                }
//            }
//        }
//    }
//}

void DMCChannel::tick() {
    if (!enabled) return;

    timer -= 1.0f;
    if (timer > 0.0f) return;
    timer += timerPeriod;

    if (bitCounter == 0) {
        if (!sampleBufferEmpty) {
            shiftRegister = sampleBuffer;
            sampleBufferEmpty = true;
            bitCounter = 8;

            std::cout << "Novo byte carregado no shiftRegister: " << (int)shiftRegister << "\n";
        }
    }

    if (bitCounter > 0) {
        if (shiftRegister & 0x01) {
            if (deltaCounter <= 125) deltaCounter += 2;
        }
        else {
            if (deltaCounter >= 2) deltaCounter -= 2;
        }

        shiftRegister >>= 1;
        bitCounter--;

        std::cout << "Bit processado. deltaCounter: " << (int)deltaCounter
            << ", shiftRegister: " << (int)shiftRegister
            << ", bits restantes: " << (int)bitCounter << "\n";
    }

    if (sampleBufferEmpty) {
        if (bytesRemaining == 0) {
            if (loopFlag) {
                restartSample();
                std::cout << "Looping sample: reiniciando endere�o e bytes\n";
            }
            else {
                // Sem loop, nada mais a fazer
                return;
            }
        }

        if (currentAddress < sampleDataSize && bytesRemaining > 0) {
            sampleBuffer = sampleData[currentAddress];
            sampleBufferEmpty = false;
            currentAddress++;
            bytesRemaining--;

            std::cout << "Sample lido da mem�ria: " << (int)sampleBuffer
                << ", endere�o: " << currentAddress - 1
                << ", restantes: " << bytesRemaining << "\n";
        }
    }

}


float DMCChannel::getSample() const {
    return enabled ? (deltaCounter / 127.0f) : 0.0f;
}

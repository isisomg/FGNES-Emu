#pragma once
#include <cstdint>

class DMCChannel {
public:
	DMCChannel();

    void tick();             // Atualiza o estado da DMC
    float getSample() const; // Retorna o volume atual
    void restartSample();
    void setEnabled(bool value);
    void setRate(uint8_t index);
    void setSampleAddress(uint8_t address);
    void setSampleLength(uint8_t length);
    void setLoop(bool loop);
    void setIRQEnabled(bool irq);
    void setSampleData(const uint8_t* data, size_t size);

    static const uint8_t fakeSample[32];

private:
    const uint8_t* sampleData = nullptr;
    size_t sampleDataSize = 0;
    uint16_t currentAddress = 0;
    uint16_t bytesRemaining = 0;
    uint8_t sampleBuffer = 0;
    bool sampleBufferEmpty = true;
    uint8_t shiftRegister = 0;
    uint8_t bitCounter = 8;

    // usa um contador delta com 7 bits que vao de 0-127, eh ajustado a cada bit lido
    uint8_t deltaCounter = 64; // Valor inicial padrão do contador delta

    float timer = 0;
    float timerPeriod;
    bool enabled = false;
    bool loopFlag = false;
    bool irqEnabled = false;
    uint16_t sampleStartAddress = 0;
    uint16_t sampleLength = 0;
};

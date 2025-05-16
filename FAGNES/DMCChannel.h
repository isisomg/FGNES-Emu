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
    uint16_t currentAddress;
    uint16_t bytesRemaining;
    uint8_t sampleBuffer;
    bool sampleBufferEmpty;
    uint8_t shiftRegister;
    uint8_t bitCounter;

    // usa um contador delta com 7 bits que vao de 0-127, eh ajustado a cada bit lido
    uint8_t deltaCounter;

    float timer;
    float timerPeriod;
    bool enabled;
    bool loopFlag;
    bool irqEnabled;
    uint16_t sampleStartAddress;
    uint16_t sampleLength;
};

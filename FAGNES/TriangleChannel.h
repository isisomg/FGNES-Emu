#pragma once
#include <cstdint>

class TriangleChannel {
public:
    void setFrequency(float freq);
    
    void setEnabled(bool on);

    float getSample() const;

    void tick(); // Avança o tempo interno
private:
    int timer = 0.0f;
    float timerPeriod = 0.0f;
    bool enabled = true;
    float frequency = 440.0f; // Frequência padrão
    int phase = 0;


    static const uint8_t triangleTable[32];
};
#pragma once
#pragma once
#include <cstdint>

class PulseChannel {
public:
    void setFrequency(float freq);
    void setVolume(int vol);
    void setDuty(int duty); // 0 = 12.5%, 1 = 25%, 2 = 50%, 3 = 75%
    void setEnabled(bool on);

    float getSample() const;

    void tick(); // Avança o tempo interno
private:
    int timer = 0.0f;
    float timerPeriod = 0.0f;
    bool enabled = true;
    float frequency = 440.0f; // Frequência padrão
    int volume = 15; //volume maximo do NES
    int duty = 2; //50%
    int phase = 0;


    static const uint8_t dutyTable[4][8];
};
#pragma once

#pragma once
#include <cstdint>
#include "Memoria.h"


class NoiseChannel {
public:
    void setFrequency(float freq);
    void setVolume(int vol);
    void setMode(bool m);
    void setEnabled(bool on);
    void setMemoria(Memoria& memoria);

    float getSample() const;

    void tick(); // avança o registrador
private:
    uint16_t shiftRegister = 1; // começa em 1 já que ao iniciar o NES, o registrador começava em 1
    int timer = 0.0f;
    float timerPeriod = 0.0f;
    bool enabled = true;
    int volume = 15; //volume maximo do NES
    bool mode = false; // no NES existem 2 modos, curto e longo, nesse caso to usando false pro longo, q eh um XOR entre o bit 0 e o 1, o bit curto eh um XOR entre o bit 0 e 6
    Memoria memoria;
};


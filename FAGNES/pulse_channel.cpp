#include "pulse_channel.h"

const uint8_t PulseChannel::dutyTable[4][8] = {

    // A tabela está definindo o formato da onda, possui um ciclo de 8 fases, sua frequencia permanece a mesma, porem seu formato muda, produzindo outros sons

    //{0,0,0,0,0,0,0,1}, // 12.5%
    //{0,0,0,0,0,1,1,1}, // 25%
    //{0,0,0,0,1,1,1,1}, // 50%
    //{1,1,1,1,0,0,0,0}  // 75%

    {0,1,0,0,0,0,0,0}, // 12.5%
    {0,1,1,0,0,1,1,1}, // 25%
    {0,1,1,1,1,0,0,0}, // 50%
    {1,0,0,1,1,1,1,1}  // 25% negado
};

//void PulseChannel::setTimer(uint16_t value) {
//    timer = value;
//}

void PulseChannel::setVolume(int vol) {
    if (vol < 0) vol = 0;
    volume = vol > 15 ? 15 : vol; // Limita o volume a 15, que é o máximo do NES
}

void PulseChannel::setDuty(int d) {
    if (d >= 0 && d < 4) { //temos apenas 4 valores na duty table, portanto d deve estar dentro desse periodo
        duty = d;
    }
}

//void PulseChannel::resetPhase() {
//    counter = 0;
//    phase = 0;
//}

void PulseChannel::tick() {

    timer -= 1.0f;
    if (timer <= 0.0f) {
        timer += timerPeriod;
        phase = (phase + 1) % 8;
    }

    /*if (++counter >= timer) {
        counter = 0;
        phase = (phase + 1) % 8;
    }*/
}

float PulseChannel::getSample() const {
    if (!enabled || dutyTable[duty][phase] == 0) return 0.0f;
    return volume / 15.0f;
}

void PulseChannel::setFrequency(float freq) {
    frequency = freq;
    timerPeriod = 44100.0f / (frequency * 8.0f); // 8 canais do duty
}

void PulseChannel::setEnabled(bool on) {
    enabled = on;
}

void PulseChannel::setMemoria(Memoria& memoria) {
    this->memoria = memoria; // isto permite que a classe NoiseChannel acesse a memória
}
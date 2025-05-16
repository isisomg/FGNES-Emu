#include "NoiseChannel.h"

void NoiseChannel::tick() {
    if (!enabled) return;

    timer -= 1.0f;
    if (timer <= 0.0f) {
        timer += timerPeriod;

        // Escolhe os bits para o XOR com base no modo (curto ou longo)
        int bit0 = shiftRegister & 1;
        int bit1or6 = (shiftRegister >> (mode ? 6 : 1)) & 1;
        int feedback = bit0 ^ bit1or6;

        // Atualiza o registrador
        shiftRegister >>= 1; // isso move os bits de valor 1 para a direita
        shiftRegister |= (feedback << 14); // coloca feedback no bit 14 
    }
}

float NoiseChannel::getSample() const {
    if (!enabled) return 0.0f;

    // se o valor do bit 0 for 0, então vai ligar, se for 1 vai silenciar
    return (shiftRegister & 1) == 0 ? (volume / 15.0f) : 0.0f;
}

void NoiseChannel::setEnabled(bool on) {
    enabled = on;
}

void NoiseChannel::setMode(bool m) {
    mode = m;
}

void NoiseChannel::setVolume(int vol) {
    if (vol < 0) vol = 0;
    volume = (vol > 15) ? 15 : vol;
}

void NoiseChannel::setFrequency(float freq) {
    if (freq <= 0.0f) freq = 1.0f; // evita divisão por zero
    timerPeriod = 44100.0f / freq;
}

void NoiseChannel::setBus(Bus* busNovo) {
    this->bus = busNovo; // isto permite que a classe NoiseChannel acesse a memória
}
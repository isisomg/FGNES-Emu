#include "APU.h"
#include <stdio.h>
#include <iostream>

const uint8_t APU::dutyTable[4][8] = {

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

const uint8_t APU::triangleTable[32]{
    15, 14, 13, 12, 11, 10,  9,  8,
    7,  6,  5,  4,  3,  2,  1,  0,
    0,  1,  2,  3,  4,  5,  6,  7,
    8,  9, 10, 11, 12, 13, 14, 15
};

//void APU::setTimer(uint16_t value) {
//    timer = value;
//}

void APU::setVolume(int vol) {
    if (vol < 0) vol = 0;
    volume = (vol > 15) ? 15 : vol; // Limita o volume a 15, que é o máximo do NES
}

void APU::setDuty(int d) {
    if (d >= 0 && d < 4) { //temos apenas 4 valores na duty table, portanto d deve estar dentro desse periodo
        duty = d;
    }
}

//void APU::resetPhase() {
//    counter = 0;
//    phase = 0;
//}


void APU::tick(int channel) { // preferi fazer por switch case pq a partir do noise muda mt a logica do tick, ent acho melhor pra alterar o modo
    if (!enabled) return;

    timer -= 1.0f;

    switch (channel) {
	case 1: // Pulse
        if (timer <= 0.0f) {
            timer += timerPeriod;
            phase = (phase + 1) % 8;
        }

        /*if (++counter >= timer) {
            counter = 0;
            phase = (phase + 1) % 8;
        }*/
        break;
	case 2: // Triangle
        if (timer <= 0.0f) {
            timer += timerPeriod;
            phase = (phase + 1) % 32;
        }
        break;
	case 3: // Noise
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
        break;
    case 4: // DMC
        if (dmcTimer <= 0.0f) {
            dmcTimer += dmcTimerPeriod;

            if (dmcBitCount == 0) {
                // Simulando carregamento de byte fixo (0x55) por enquanto
                dmcShiftReg = 0x55;
                dmcBitCount = 8;
            }

            uint8_t bit = dmcShiftReg & 1;

            if (bit) {
                if (dmcOutputLevel <= 125) dmcOutputLevel += 2;
            }
            else {
                if (dmcOutputLevel >= 2) dmcOutputLevel -= 2;
            }

            dmcShiftReg >>= 1;
            dmcBitCount--;
        }
        else {
            dmcTimer -= 1.0f;
        }
        break;
	default:
		std::cerr << "Canal inválido: " << channel << "\n";
		return; // Canal inválido  
        break;
    }
}

float APU::getSample(int channel) const {
    switch (channel) {
	    case 1: // Pulse
            if (!enabled || dutyTable[duty][phase] == 0) return 0.0f;
            return volume / 15.0f;
            break;
	    case 2: // Triangle
            if (!enabled) return 0.0f;
            return triangleTable[phase] / 15.0f; // não tem volume ajustável, portanto o valor é normalizado pela fase da onda
            break;
	    case 3: // Noise
            if (!enabled) return 0.0f;

            // se o valor do bit 0 for 0, então vai ligar, se for 1 vai silenciar
            return (shiftRegister & 1) == 0 ? (volume / 15.0f) : 0.0f;
            break;
        case 4: // DMC
            if (!enabled) return 0.0f;
            return dmcOutputLevel / 127.0f;

	    default:
		    return 0.0f; // Canal inválido
            break;
    }
}

void APU::setFrequency(float freq, float can) {
    canais = can;
    frequency = freq;
    if (frequency <= 0.0f) frequency = 1.0f; // evita divisão por zero
	timerPeriod = 44100.0f / (frequency * canais); // 8 canais do duty, 32 canais do triangles, 1 por padrão do noise
}

void APU::setEnabled(bool on) {
    enabled = on;
}

void APU::setBus(Bus* busNovo) {
    this->bus = busNovo; 
}

void APU::setMode(bool m) {
    mode = m;
}


#pragma once
#include <cstdint>
#include "Bus.h"

// fazendo sem o DMC por enquanto

class APU {
public:
	void setFrequency(float freq, float can); // pulse, triangle, noise
	void setVolume(int vol); // pulse, noise
    void setDuty(int duty); // 0 = 12.5%, 1 = 25%, 2 = 50%, 3 = 75% (pulse)
	void setMode(bool m); // false = modo longo, true = curto (noise)
    void setEnabled(bool on); // todos
    void setBus(Bus* memoria); // todos

	float getSample(int channel) const; // todos (1 = pulse, 2 = triangle, 3 = noise, 4 = DMC)

    void tick(int channel); // Avança o tempo interno (todos usam) (1 = pulse, 2 = triangle, 3 = noise, 4 = DMC)
private:

	uint16_t shiftRegister = 1; // começa em 1 já que ao iniciar o NES, o registrador começava em 1 (noise)
	int timer = 0.0f; // todos usam (o dmc usa float, vou ver como arrumar)
    float timerPeriod = 0.0f; // pulse, triangle, noise
    bool enabled = true; // pulse, triangle, noise
    float frequency = 440.0f; // Frequência padrão (pulse, triangle usam)
	float canais = 1.0f; // canal padrao, pulse, triangle, noise
	int volume = 15; //volume maximo do NES (pulse, noise)
	int duty = 2; //50% (pulse)
	int phase = 0; // pulse, triangle
	bool mode = false; // no NES existem 2 modos, curto e longo, nesse caso to usando false pro longo, q eh um XOR entre o bit 0 e o 1, o bit curto eh um XOR entre o bit 0 e 6 (noise)
    Bus* bus; // todos

	static const uint8_t dutyTable[4][8]; // pulse
	static const uint8_t triangleTable[32]; // triangle

	// DMC
	uint8_t dmcShiftReg = 0x00;
	uint8_t dmcBitCount = 0;
	uint8_t dmcOutputLevel = 0;
	float dmcTimer = 0.0f;
	float dmcTimerPeriod = 428.0f; // Valor comum para taxa 44100 Hz / 428 ? 103.1 Hz

};
#pragma once

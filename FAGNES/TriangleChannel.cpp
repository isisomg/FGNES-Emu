#include "TriangleChannel.h"

const uint8_t TriangleChannel::triangleTable[32]{
	15, 14, 13, 12, 11, 10,  9,  8,  
	7,  6,  5,  4,  3,  2,  1,  0,
	0,  1,  2,  3,  4,  5,  6,  7,  
	8,  9, 10, 11, 12, 13, 14, 15
};

void TriangleChannel::tick() {
	if (!enabled) return;

	timer -= 1.0f;
	if (timer <= 0.0f) {
		timer += timerPeriod;
		phase = (phase + 1) % 32;
	}
}

float TriangleChannel::getSample() const { // retorna o valor atual da onda
	if (!enabled) return 0.0f;
	return triangleTable[phase] / 15.0f; // não tem volume ajustável, portanto o valor é normalizado pela fase da onda
}

void TriangleChannel::setFrequency(float freq) {
	timerPeriod = 44100.0f / (freq * 32.0f); // 32 canais da tabela
}

void TriangleChannel::setEnabled(bool on) {
	enabled = on;
}

void TriangleChannel::setBus(Bus* busNovo) {
	this->bus = busNovo; // isto permite que a classe NoiseChannel acesse a memória
}
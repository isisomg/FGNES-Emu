#pragma once

#include <cstdint>

//índices dos botões
enum botoesNES {

	A = 0,
	B = 1,
	SELECT = 2,
	START = 3,
	UP = 4,
	DOWN = 5,
	LEFT = 6,
	RIGHT = 7

};

//Classe controles
class Controles {

private:
	uint8_t estado = 0;
	uint8_t indice = 0;
	bool strobe = false;

public:
	void pressionar(botoesNES botao);
	void soltar(botoesNES botao);

	void escreverStrobe(bool valor);
	uint8_t ler();
};
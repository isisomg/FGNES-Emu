#pragma once
#include "Tipos.h"

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
	Byte estado = 0;
	Byte indice = 0;
	bool strobe = false;
	
public:
	void pressionar(botoesNES botao);
	void soltar(botoesNES botao);

	void escreverStrobe(bool valor);
	Byte ler();
};
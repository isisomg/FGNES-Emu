#pragma once
#include "Tipos.h"

template <DWord Tamanho>

class Memoria {
private:
	Byte memoriaTotal[Tamanho] = { 0x0 };

public:
	// Leitura (read-only)
	Byte operator[](DWord address) const {
		if (address >= Tamanho) { // Fora do endereco acessivel
			return 0xFF; // Retorna um valor padrao para evitar crash
		}
		return memoriaTotal[address];
	}

	// Escrita (write)
	Byte& operator[](DWord address) {
		if (address >= Tamanho) { // Fora do enndereco acessivel
			return memoriaTotal[0x0000]; // valor padrao para evitar crash
		}
		return memoriaTotal[address];
	}
};

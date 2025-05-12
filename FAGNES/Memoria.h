#pragma once

using Byte = unsigned char;
using DWord = unsigned short;

constexpr DWord SIZE_MEMORIA = 0xFFFF;

struct Memoria {
	Byte memoriaTotal[SIZE_MEMORIA] = { 0x0 };

	// Leitura (read-only)
	Byte operator[](DWord address) const {
		return memoriaTotal[address];
	}

	// Escrita (write)
	Byte& operator[](DWord address) {
		return memoriaTotal[address];
	}
};

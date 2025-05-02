#pragma once

using Byte = unsigned char;
using DWord = unsigned short;

#define SIZE_MEMORIA 0xFFFF + 0x1


struct Memoria
{

	Byte memoriaTotal[SIZE_MEMORIA] = { 0x0 };

	// Read
	Byte operator[](DWord address) const {
		return memoriaTotal[address];
	}

	// Write
	Byte& operator[](DWord address) {
		return memoriaTotal[address];
	};

};
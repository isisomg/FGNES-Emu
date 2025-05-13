#pragma once
#include "Memoria.h"

using Byte = unsigned char;
using DWord = unsigned short;

class Bus {
private:
	Memoria mem;

public:
	void write(DWord adr, Byte dado);
	Byte read(DWord adr);
};
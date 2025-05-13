#pragma once
#include "Memoria.h"

using Byte = unsigned char;
using DWord = unsigned short;

class Bus {
private:
	Memoria<0x0800> memCPU;
	Memoria<0x4000> memPPU;

public:
	void write(DWord adr, Byte dado);
	Byte read(DWord adr);
};
#pragma once
#include "Memoria.h"
#include "Cartucho.h"
#include "Controles.h"
#include "APU.h"

class APU;

class Bus {
private:
	APU* apu = nullptr;
	Memoria<0x0800> memCPU;
	Memoria<0x4000> memPPU;
	Cartucho* cartucho = nullptr;
	Controles controles;

public:
	void setAPU(APU* novaAPU);
	bool checkNMI();
	void setCartucho(Cartucho* cart);
	void write(DWord adr, Byte dado);
	Byte read(DWord adr);

	Controles* getControles() { return &controles; };
};
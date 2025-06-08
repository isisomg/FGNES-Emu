#pragma once
#include "Memoria.h"
#include "Cartucho.h"
#include "Controles.h"
#include "APU.h"
#include "PPU.h" //adicionei a ppu haha!!!:D

class APU;

class Bus {
private:
	PPU* ppu = nullptr;
	APU* apu = nullptr;
	Memoria<0x0800> memCPU;
	Memoria<0x4000> memPPU;
	Cartucho* cartucho = nullptr;
	Controles* controlesP1 = nullptr;
	Controles* controlesP2 = nullptr;

public:
	void setPPU(PPU* novaPPU);
	void setAPU(APU* novaAPU);
	bool checkNMI();
	void setCartucho(Cartucho* cart);
	void write(DWord adr, Byte dado);
	Byte read(DWord adr);
	void setControlesP1(Controles* controle);
	void setControlesP2(Controles* controle);
	Controles* getControlesP1() const { return controlesP1; };
	Controles* getControlesP2() const { return controlesP2; };
};
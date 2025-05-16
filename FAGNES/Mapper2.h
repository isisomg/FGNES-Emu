#pragma once
#include "Mapper.h"
#include "Tipos.h"
#include <vector>
#include <iostream>

class Mapper2 : public Mapper {
private:
	Byte prgBankSelect = 0;
	std::vector<Byte> prgROM;
	std::vector<Byte> chrROM;
	int prgBanks;
	int chrBanks;

public:
	Mapper2(Byte prgBanks, Byte chrBanks, const std::vector<Byte>& prg, const std::vector<Byte>& chr);

	Byte cpuRead(DWord addr) override;
	void cpuWrite(DWord addr, Byte data) override;

	Byte ppuRead(DWord addr) override;
	void ppuWrite(DWord addr, Byte data) override;
};

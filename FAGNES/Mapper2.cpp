#include "Mapper2.h"
#include <iostream>

Mapper2::Mapper2(Byte prgBanks_param, Byte chrBanks_param, const std::vector<Byte>& prg, const std::vector<Byte>& chr_data_from_cart)
	: prgROM(prg),
	chrROM(chr_data_from_cart),
	prgBanks(prgBanks_param),
	chrBanks(chrBanks_param)
{
	if (this->chrBanks == 0) {
		this->chrROM.clear();
		this->chrROM.resize(0x2000);
	}
}

Byte Mapper2::cpuRead(DWord addr) {
	if (addr >= 0x8000 && addr <= 0xBFFF) {
		int bankOffset = prgBankSelect * 0x4000;
		return prgROM[bankOffset + (addr - 0x8000)];
	}
	else if (addr >= 0xC000 && addr <= 0xFFFF) {
		if (prgBanks > 0) {
			int bankOffset = (prgBanks - 1) * 0x4000;
			return prgROM[bankOffset + (addr - 0xC000)];
		}
	}
	return 0x00;
}

void Mapper2::cpuWrite(DWord addr, Byte data) {
	if (addr >= 0x8000 && addr <= 0xFFFF) {
		if (prgBanks > 0) { // Evita divisão por zero ou módulo por zero
			prgBankSelect = (data & 0x0F) % prgBanks;
		}
		else {
			prgBankSelect = 0; // Comportamento de fallback seguro
		}
	}
}

Byte Mapper2::ppuRead(DWord addr) {
	if (addr < 0x2000) {
		if (!chrROM.empty()) {
			return chrROM[addr & 0x1FFF];
		}
	}
	return 0x00;
}

void Mapper2::ppuWrite(DWord addr, Byte data) {
	if (addr < 0x2000) {
		if (chrBanks == 0 && !chrROM.empty()) {
			chrROM[addr & 0x1FFF] = data;
		}
		else {
			if (chrBanks > 0) {
				std::cerr << "Mapper2 não suporta escrita na CHR-ROM" << std::endl;
			}
		}
	}
}
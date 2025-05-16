#include "Mapper2.h"

Mapper2::Mapper2(Byte prgBanks, Byte chrBanks, const std::vector<Byte>& prg, const std::vector<Byte>& chr)
	: prgROM(prg), chrROM(chr), prgBanks(prgBanks), chrBanks(chrBanks) {
	
}

Byte Mapper2::cpuRead(DWord addr) {
	if (addr >= 0x8000 && addr <= 0xBFFF) {
		// Banco selecionado dinamicamente
		int bankOffset = prgBankSelect * 0x4000;
		return prgROM[bankOffset + (addr - 0x8000)];
	}
	else if (addr >= 0xC000 && addr <= 0xFFFF) {
		// Último banco sempre fixo
		int bankOffset = (prgBanks - 1) * 0x4000;
		return prgROM[bankOffset + (addr - 0xC000)];
	}
	return 0x00;
}

void Mapper2::cpuWrite(DWord addr, Byte data) {
	if (addr >= 0x8000 && addr <= 0xFFFF) {
		prgBankSelect = data & 0x0F; // só 4 bits válidos
	}
}

Byte Mapper2::ppuRead(DWord addr) {
	if (addr < 0x2000) {
		return chrROM[addr];
	}
	return 0x00;
}

void Mapper2::ppuWrite(DWord addr, Byte data) {
	std::cerr << "Mapper2 não suporta escrita na CHR-ROM\n";
}
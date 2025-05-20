#include "Mapper0.h"

Mapper0::Mapper0(Byte prg_Banks, Byte chr_Banks, const std::vector<Byte>& prg, const std::vector<Byte>& chr) {
	this->prgBanks = prg_Banks;
	this->chrBanks = chr_Banks;
	this->prgROM = prg;
	this->chrROM = chr;
}

Byte Mapper0::cpuRead(DWord addr) {
	if (addr >= 0x8000 && addr <= 0xFFFF) {
		if (prgBanks == 1) {
			// 16KB espelhado: 0x8000–0xBFFF e 0xC000–0xFFFF são o mesmo banco
			return prgROM[addr & 0x3FFF]; // 16KB = 0x4000
		}
		else {
			// 32KB: acessa direto
			return prgROM[addr - 0x8000];
		}
	}
	return 0x00;
}


Byte Mapper0::ppuRead(DWord addr) {
	if (addr < 0x2000) {
		return chrROM[addr];
	}
	return 0x00;
}

void Mapper0::cpuWrite(DWord adr, Byte dado) {
	std::cerr << "Indisponivel no mapper0\n";
}
void Mapper0::ppuWrite(DWord addr, Byte data) {
	std::cerr << "Indisponível no Mapper0\n";
}
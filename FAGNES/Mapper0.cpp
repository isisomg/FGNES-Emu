#include "Mapper0.h"

Mapper0::Mapper0(Byte prg_Banks, Byte chr_Banks, const std::vector<Byte>& prg, const std::vector<Byte>& chr) {
	this->prgBanks = prg_Banks;
	this->chrBanks = chr_Banks;
	this->prgROM = prg;
	this->chrROM = chr;
}

Byte Mapper0::cpuRead(DWord adr) {
	if (adr >= 0x8000) { // adr da pgr
		if (prgBanks > 0) { // se tem bancos
			return prgROM[adr & 0x3FFF]; // tira espelhamento
		}
		else { // Unico banco
			return prgROM[adr & 0x7FFF]; // tira espelhamento
		}
	}
	return 0x00; // endereco invalido, retorna qualquer coisa
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
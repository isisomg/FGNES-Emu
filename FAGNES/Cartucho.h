#pragma once
#include <vector>
#include <iostream>
#include "Tipos.h"
#include <string>
#include <fstream>
#include "Mapper.h"
#include "Mapper0.h"

class Cartucho {
private:
	std::vector<Byte> prgROM;
	
	std::vector<Byte> prgRAM;   // PRG-RAM (Save RAM)
	std::vector<Byte> chrRAM;   // CHR-RAM (se não houver CHR-ROM)
	// Mapper quando tiver aqui
	std::unique_ptr<Mapper> mapper;
	
public:
	std::vector<Byte> chrROM;
	DWord adrPCinicial = 0x0000;
	void init(const std::string& pathROM); // Define valores padroes e carrega a rom
	Byte readPRG(DWord adr);
	void writePRG(DWord adr, Byte dado);
	Byte readCHR(DWord adr);
	void writeCHR(DWord adr, Byte dado);
};

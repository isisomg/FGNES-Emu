#pragma once
#include <vector>
#include <iostream>
#include "Tipos.h"
#include <string>
#include <fstream>

class Cartucho {
private:
	std::vector<Byte> prgROM;
	std::vector<Byte> chrROM;
	std::vector<Byte> prgRAM;   // PRG-RAM (Save RAM)
	std::vector<Byte> chrRAM;   // CHR-RAM (se não houver CHR-ROM)
	// Mapper quando tiver aqui
public:

	void init(const std::string& pathROM); // Define valores padroes e carrega a rom
	Byte readPRG();
	void writePRG();
	Byte readCHR();
	void writeCHR();
};

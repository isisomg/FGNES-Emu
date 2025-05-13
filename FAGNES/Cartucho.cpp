#include "Cartucho.h"

void Cartucho::init(const std::string& path) {
	std::ifstream arquivo(path, std::ios::binary);

	if (!arquivo) { // Erro ao carregar ROM
		std::cerr << "Erro ao carregar arquivo .nes" << std::endl;
		exit(1);
	}

	// Lendo cabecalho arquivo nes16 bytes
	char header[16];
	arquivo.read(header, 16);
	if (header[0] != 'N' || header[1] != 'E' || header[2] != 'S') { // magic numbers da rom
		std::cerr << "Formato .nes invalido" << std::endl;
		exit(2);
	}

	int prgBanks = header[4]; // quantidade de bancos prg, sendo cada um deles de 16kb.
	int chrBanks = header[5]; // quantidade de bancos chr, sendo cada um deles de 8kb.

	bool saveRAM = (header[6] & 0x02) != 0;  // verifica se tem funcionalidade de save no cartucho

	Cartucho::prgROM.resize(prgBanks * 0x4000); // aloca memoria para os bancos prg
	arquivo.read(reinterpret_cast<char*>(Cartucho::prgROM.data()), Cartucho::prgROM.size());

	if (chrBanks > 0) { // aloca memoria para bancos chr
		Cartucho::chrROM.resize(chrBanks * 0x2000);
		arquivo.read(reinterpret_cast<char*>(Cartucho::chrROM.data()), Cartucho::chrROM.size());
	}
	else { // se nao tem banco, aloca como ram
		Cartucho::chrRAM.resize(0x2000);
	}
	
	arquivo.close();

	// Inicializar os mappers quando tiver.
}

// REQUER MAPPERS PARA IMPLEMENTAR
//Byte Cartucho::readCHR(DWord adr) {
//	if (!Cartucho::chrROM.empty()) { // usa ROM
//		return 
//	}
//	else // usa RAM
//	{
//		return 
//	}
//}
//void Cartucho::writeCHR(DWord adr) { 
//	if (!chrRAM.size() > 0) { // apenas se for RAM
//
//	}
//}
//Byte Cartucho::readCHR(DWord adr) {
//	return 
//}
//void Cartucho::writeCHR(DWord adr) {
//	
//}
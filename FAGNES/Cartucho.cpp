#include "Cartucho.h"
#include "Mapper2.h"
#include "Bus.h"
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

	prgROM.resize(prgBanks * 0x4000); // aloca memoria para os bancos prg
	arquivo.read(reinterpret_cast<char*>(prgROM.data()), prgROM.size());

	if (chrBanks > 0) { // aloca memoria para bancos chr
		chrROM.resize(chrBanks * 0x2000);
		arquivo.read(reinterpret_cast<char*>(chrROM.data()), chrROM.size());
	}
	else { // se nao tem banco, aloca como ram
		chrRAM.resize(0x2000);
	}
	
	arquivo.close();
	
	
	
	// Cálculo do número do mapper
	Byte mapperLow = (header[6] >> 4) & 0x0F;
	Byte mapperHigh = (header[7] >> 4) & 0x0F;
	Byte valorMapper = (mapperHigh << 4) | mapperLow;

	// Criar o Mapper correto
	switch (valorMapper) {
	case 0:
		mapper = std::make_unique<Mapper0>(prgBanks, chrBanks, prgROM, chrROM); //NROM MAPPER 0
		adrPCinicial = prgROM[0x7FFC] | (prgROM[0x7FFD] << 8);
		break;
	case 1:
		//mapper = std::make_unique<Mapper1>(prgBanks, chrBanks, prgROM, chrROM); // MMC1 MAPPER1
		break;
	case 2:
		mapper = std::make_unique<Mapper2>(prgBanks, chrBanks, prgROM, chrROM); // UxROM MAPPER2
		adrPCinicial = readPRG(0xFFFC) | (readPRG(0xFFFD) << 8);
		break;
	default:
		std::cerr << "Mapper " << (int)valorMapper << " não suportado ainda.\n";
		exit(3);
	}


	std::cout << "Jogo carregado. Mapper" << (int)valorMapper << " sendo usado. PC inicial: " << std::hex << (int)adrPCinicial << std::endl;
}

Byte Cartucho::readPRG(DWord adr) {
	return mapper->cpuRead(adr);
}
void Cartucho::writePRG(DWord adr, Byte dado) { 
	mapper->cpuWrite(adr, dado);
}
Byte Cartucho::readCHR(DWord adr) {
	return mapper->ppuRead(adr);
}
void Cartucho::writeCHR(DWord adr, Byte dado) {
	mapper->ppuWrite(adr, dado);
}
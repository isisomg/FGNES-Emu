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

	// Pegando o mapper correto
	Byte valorMapper = header[7];
	if (valorMapper == 0) { // mapper 0 
		mapper = std::make_unique<Mapper0>(prgBanks, chrBanks, prgROM, chrROM);
		adrPCinicial = prgROM[prgROM.size() - 6] | (prgROM[prgROM.size() - 5] << 8); // endereco PC inicial ARRUMAR
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
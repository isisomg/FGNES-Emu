#include "PPU.h"
#include "Tipos.h"

//Somente pra testes, descomentar se nao for testar!!
#include <iostream>


//////////////////////////////////////////////////////
//                    PPUCTRL                       //
//////////////////////////////////////////////////////


void PPUCTRL::write(Byte value) {
	control = value;
	// Haha inteiro de 8 bits que é de 0 a 255, o NES usa registradores de 8 bits ent é isso que vamos usar!!
	// Quando a CPU escrever no registrador $2000, o PPUCTRL é atualizado, vamos guardar esse valor na variável control ta???
	// EU TO ESCREVENDO ISSO PQ EU ACHO Q É IMPORTANTE ENTENDER COMO FUNCIONA.
	// ASSINADO: ISIS. >:(
}

bool PPUCTRL::isNMIEnabled() const {
	// Pelo que entendi, na CPU vai ter que chamar essa função, e ativar o NMI lá.
	// ASSINADO: ISIS. >:(
	return (control & 0x80) != 0;
}

bool PPUCTRL::isMasterSlave() const {
	// Esse aqui é só um getter, mas ele vai retornar o valor do bit 5 do control.
	// ASSINADO: ISIS. >:(
	return (control & 0x40) != 0;
}

Byte PPUCTRL::getNameTableAddr() const {

	// Esse aqui também é só um getter, mas ele vai retornar os bits 0-1.
	// ASSINADO: ISIS. >:(
	return control & 0x03;
}


// ESTOU PARANDO DE ASSINAR!!!!!


//////////////////////////////////////////////////////
//                    PPUSTATUS                     //
//////////////////////////////////////////////////////


Byte PPUSTATUS::read() {
	// Aqui ele vai retornar o valor do status, mas resetar o bit de VBlank (bit 7).
	Byte result = status;
	status &= ~0x80;
	return result;
}

void PPUSTATUS::setVBlank(bool value) {
	// Aqui ele vai setar o bit 7 do status, que é o VBlank.
	if (value) {
		status |= 0x80;
	}
	else {
		status &= ~0x80;
	}
}

//////////////////////////////////////////////////////
//                    VRAM                          //
//////////////////////////////////////////////////////

Byte PPU::read(DWord address) {
	address &= 0x3FFF;

	if (address >= 0x2000 && address <= 0x3EFF) {
		// Nossas lindas e maravilhosas nametables (com espelhamento)
		DWord mirroredAddress = mirrorAddress(address);
		return nametableVRAM[mirroredAddress];
	}

	if (address < 0x2000) {
		// Pattern Tables
		return patternTable[address];
	}

	if (address >= 0x3F00 && address <= 0x3FFF) {
		// Paleta
		DWord paletteAddress = (address - 0x3F00) % 32;
		return paletteRAM[paletteAddress];
	}
	return 0x00;
}

void PPU::write(DWord address, Byte value) {
	address &= 0x3FFF;

	if (address < 0x2000) {
		// Pattern tables
		patternTable[address] = value;
	}
	else if (address >= 0x2000 && address <= 0x3EFF) {
		// Nametables (com espelhamento)
		DWord mirroredAddress = mirrorAddress(address);
		nametableVRAM[mirroredAddress] = value;
	}
	else if (address >= 0x3F00 && address <= 0x3FFF) {
		// Paleta
		DWord paletteAddress = (address - 0x3F00) % 32;
		paletteRAM[paletteAddress] = value;
	}
}

Byte PPU::readFromPPUData() {
	Byte value = read(ppuAddress);

	// Lógica de buffer: somente paleta é lida diretamente
	if (ppuAddress < 0x3F00) {
		Byte buffered = ppuDataBuffer;
		ppuDataBuffer = value;
		value = buffered;
	}
	else {
		// Lê diretamente da paleta (sem buffer delay)
		ppuDataBuffer = read(ppuAddress - 0x1000); // efeito colateral do NES
	}

	// Incrementa endereço (bit 2 de PPUCTRL define passo)
	ppuAddress += (ctrl.control & 0x04) ? 32 : 1;
	return value;
}

void PPU::writeToPPUData(Byte value) {
	write(ppuAddress, value);

	// Incrementa endereço
	ppuAddress += (ctrl.control & 0x04) ? 32 : 1;
}


DWord PPU::mirrorAddress(DWord address) {
	// Faz o espelhamento de nametables (para se ajustar ao tamanho real da RAM de 2kb)
	address = (address - 0x2000) % 0x1000;
	return address % 0x800;
}

//////////////////////////////////////////////////////
//            VBLANK (FEITO) & STEP                 //										uhul!
//////////////////////////////////////////////////////

bool nmiRequested = false;

void (*nmiCallback)() = nullptr;

void PPU::step() {
	dot++;
	// O DOT eh de 0 a 340 (341 pontos por linha) !!!
	if (dot > 340) {
		dot = 0;
		scanline++;

		// O SCANLINE eh de 0 a 261 (262 linhas por frame) !!!
		if (scanline > 261) {
			scanline = 0;
		}
	}

	if (scanline == 241 && dot == 1) {
		status.setVBlank(true);
		if (ctrl.isNMIEnabled()) {
			nmiRequested = true;
			if (nmiCallback) nmiCallback();
		}
		// Sinaliza que um frame foi concluído
		
	}

	if (scanline == 261 && dot == 1) {
		// Pré-render: limpa o VBlank
		status.setVBlank(false);
	}

	if (scanline < 240) {
		// Desenha sprites
		//renderScanline(scanline);
		;
	}
}

// VAI VERIFICAR SE PRECISA DO NMI
bool PPU::isNMIRequested() {
	bool result = nmiRequested;
	nmiRequested = false;
	return result;
}

//////////////////////////////////////////////////////
//           Renderização de Sprites                //
//////////////////////////////////////////////////////

void PPU::renderScanline(int scanline) {
	renderSprites(scanline);
}

void PPU::drawSpriteTile(Byte tileIndex, Byte x, Byte y, Byte attributes, int scanline) {
	// ! ! ! P L A C E H O L D E R ! ! !  para desenhar um tile do sprite ! ! !
}

void PPU::renderSprites(int scanline) {
	// Informações em: https://www.nesdev.org/wiki/PPU_OAM

	// Sprite Height depende de config, aqui 8 como padrão

	int spriteHeight = 8;
	int spritesOnThisLine = 0;

	for (int i = 0; i < 64; ++i) {
		Byte y = OAM[i * 4 + 0];
		Byte tileIndex = OAM[i * 4 + 1];
		Byte attributes = OAM[i * 4 + 2];
		Byte x = OAM[i * 4 + 3];

		if (scanline >= y + 1 && scanline < y + 1 + spriteHeight) {
			if (spritesOnThisLine >= 8) break;
			drawSpriteTile(tileIndex, x, y + 1, attributes, scanline);
			spritesOnThisLine++;
		}
	}
}

//////////////////////////////////////////////////////
//                OAM DMA & Registradores           //
//////////////////////////////////////////////////////

void PPU::doOAMDMA(const Byte* cpuMemoryPage) {
	for (int i = 0; i < 256; ++i) {
		OAM[i] = cpuMemoryPage[i];
	}
}

void PPU::writeToPPUADDR(Byte value) {
	// $2006: escreve endereço de 16 bits em duas etapas
	if (!addressLatch) {
		ppuAddress = (value << 8) | (ppuAddress & 0x00FF);
		addressLatch = true;
	}
	else {
		ppuAddress = (ppuAddress & 0xFF00) | value;
		addressLatch = false;
	}
}

//////////////////////////////////////////////////////
//                Interface com CPU                 //
//////////////////////////////////////////////////////

void PPU::cpuWrite(DWord addr, Byte data) {
	switch (addr & 0x0007) {
	case 0x0:	// $2000 - PPUCTRL
		ctrl.write(data);
		break;
	case 0x3:	// $2003 - OAMADDR
		oamAddress = data;
		break;
	case 0x4:	// $2004 - OAMDATA
		OAM[oamAddress] = data;
		oamAddress++;
		break;
	case 0x6:	// $2006 - PPUADDR
		writeToPPUADDR(data);
		break;
	case 0x7: // $2007 - PPUDATA
		writeToPPUData(data);
		break;
	default:
		break;
	}
}

Byte PPU::cpuRead(DWord addr) {
	Byte data = 0x00;
	switch (addr & 0x0007) {
	case 0x2:	// $2002 - PPUSTATUS
		data = status.read();
		addressLatch = false;
		break;
	case 0x4:	// $2004 - OAMDATA
		data = OAM[oamAddress];
		break;
	case 0x7: // $2007 - PPUDATA
		data = readFromPPUData();
		break;
	default:
		break;

	}
	return data;
}

//////////////////////////////////////////////////////////////////////////////////////////		FALTA:  	 ////////////////////////////////////////////////////////////////////////////////////////////////////

//				COISAS QUE FALTAM E QUE PRECISAM PRO FAGNES SER FUNCIONAVEL:
// 
//	Coiso							Importancia						Descricao
// Pattern tables($0000 - $1FFF)		Alta		Sem isso nao da pra renderizar tiles.
// Paletas($3F00 - $3FFF)				Alta		Precisa para cor real na tela.
// Renderização real do background		Alta		Precisa buscar tiles, atributos e desenhar.
// Sprites(OAM) na tela					Media		Mas necessario para jogos funcionarem.
// Scroll($2005, $2006)					Media		Fundos moveis exigem isso.
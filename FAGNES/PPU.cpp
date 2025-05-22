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

void PPU::carregarCHR(const std::vector<Byte>& chrData) {
	/*if (chrData.size() > sizeof(patternTable)) {
		std::cerr << "Erro: CHR-ROM maior que 8KB!" << std::endl;
		return;
	}*/

	// Copia os dados da CHR-ROM para a tabela de padrões
	std::copy(chrData.begin(), chrData.end(), patternTable);
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
	ppuAddress = (ppuAddress + ((ctrl.control & 0x04) ? 32 : 1)) & 0x3FFF;
	return value;
}

void PPU::writeToPPUData(Byte value) {
	write(ppuAddress, value);

	// Incrementa endereço
	ppuAddress += (ctrl.control & 0x04) ? 32 : 1;
}



//////////////////////////////////////////////////////
//			        MIRRORING!!						//									EBAAAAA!
//////////////////////////////////////////////////////

// Da pra melhorar o Mirroring com certeza, tem jogos do NES que usam FOUR SCREEN MIRRORING (brutal e medonho)
// Mas sao tipo POUQUISSIMOS jogos.
// Lista deles:
// 
//  Rad Racer II.
//	Gauntlet.
//	Napoleon Senki.																	(É O NOSSO AMIGO!!!!)
//	Rocman X(Sachen) 																(Que é um bootleg)
//	Todos os jogos Vs.System. que sao basicamente uma versao arcade do NES.

MirroringSelect mirroringselect = MirroringSelect::Horizontal;

DWord PPU::mirrorAddress(DWord address) {
	address = (address - 0x2000) % 0x1000; // Só parte da nametable (0x2000~0x2FFF)

	DWord table = address / 0x400; // 0, 1, 2, 3 (nametable lógica)
	DWord offset = address % 0x400;

	switch (mirroringselect) {
	case MirroringSelect::Vertical:
		// 0 e 2 → NT0, 1 e 3 vai fica NT1
		return (table % 2) * 0x400 + offset;

	case MirroringSelect::Horizontal:
		// 0 e 1 → NT0, 2 e 3 vai fica NT1
		return (table / 2) * 0x400 + offset;
	}

	return offset; // fallback (não deveria acontecer)
}

//	NO CASO DESSE MIRRORING TER DADO ERRADO AQUI VAI FICAR O CODIGO DO MIRRORING MEIO FALSO QUE FIZ:

//DWord PPU::mirrorAddress(DWord address) {
//	address = (address - 0x2000) % 0x1000;
//	return address % 0x800; // Mirroring simplificado que fiz, ainda nao muito bem implementado! Vamos retorar aqui dps.
//}



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

	if (scanline < 240 && dot == 0) {
		renderBackgroundScanline(scanline);
		checkSpriteZeroHit(scanline);
		renderScanline(scanline); // aqui chama os sprites
	}

	if (scanline == 0) {
		std::fill(backgroundBuffer, backgroundBuffer + 256 * 240, 0);

	}

	// SPRITE 0-HIT EBA EBA
}

void PPU::renderBackgroundScanline(int scanline) {
	bool usePatternTable1 = (ctrl.control & 0x10);
	DWord patternBase = usePatternTable1 ? 0x1000 : 0x0000;
	DWord nametableBase = 0x2000 + (ctrl.control & 0x03) * 0x400;

	int tileY = scanline / 8;
	int rowInTile = scanline % 8;

	// Cache da paleta
	Byte paleta[32];
	for (int i = 0; i < 32; ++i)
		paleta[i] = read(0x3F00 + i);

	for (int tileX = 0; tileX < 32; ++tileX) {
		DWord tileIndexAddr = nametableBase + tileY * 32 + tileX;
		Byte tileIndex = read(tileIndexAddr);

		// Atributos
		int attrX = tileX / 4;
		int attrY = tileY / 4;
		DWord attributeAddr = nametableBase + 0x3C0 + attrY * 8 + attrX;
		Byte attributeByte = read(attributeAddr);

		int shift = ((tileY % 4) / 2) * 4 + ((tileX % 4) / 2) * 2;
		Byte paletteBits = (attributeByte >> shift) & 0x03;

		DWord tileAddr = patternBase + tileIndex * 16 + rowInTile;
		Byte low = read(tileAddr);
		Byte high = read(tileAddr + 8);

		for (int pixel = 0; pixel < 8; ++pixel) {
			int bit = 7 - pixel;
			Byte bit0 = (low >> bit) & 1;
			Byte bit1 = (high >> bit) & 1;
			Byte colorIndex = (bit1 << 1) | bit0;
			if (colorIndex == 0) continue;

			Byte paletteIndex = paletteBits * 4 + colorIndex;
			Byte color = paleta[paletteIndex & 0x1F];

			int x = tileX * 8 + pixel;
			int offset = scanline * 256 + x;
			Pixel cor = cores[color % 64];
			framebuffer[offset] = (0xFF << 24) | (cor.r << 16) | (cor.g << 8) | cor.b;

			// Atualiza o buffer de opacidade para sprite 0-hit
			backgroundBuffer[offset] = (colorIndex == 0) ? 0 : 1;

		}
	}
}


// VAI VERIFICAR SE PRECISA DO NMI
bool PPU::isNMIRequested() {
	bool result = nmiRequested;
	nmiRequested = false;
	return result;
}

// Começo do:
//////////////////////////////////////////////////////
//                   SCROLLER                       //
//////////////////////////////////////////////////////

// Vou colocar isso no h, soq meu vscode bugou e nao ta salvando o h, entao vou colocar aqui mesmo, mas depois eu coloco no h de novo.

//DWord v = 0; // Esse é o endereço de vram que ta sendo usado
//DWord t = 0; // Endereco de VRAM temporario, que, PASMEM, vamos usar pra fazer o scroll! Eba!
//DWord x = 0;  // X é o offset do tile que ta sendo renderizado. Ele vai de 0 a 7, e depois volta pra 0.
//bool w = false; // write toggle !!! Isso é um latch que alterna entre os dois registradores de endereço, o t e o v. Ele é usado pra fazer scroll horizontal.
//
//void writePPUScroll(Byte value) {
//	if (!w) {
//		// A primeira escrita vai pro scroll x
//		t = (t & 0x7FE0) | (value >> 3); // coarse X (5 bits) ((oraculo))
//		x = value & 0x07;                // fine X (3 bits)	  ((oraculo))
//		w = true;
//	}
//	else {
//		// A segunda escrita vai pro scroll y
//		t = (t & 0x0C1F) | ((value & 0x07) << 12) | ((value & 0xF8) << 2); //  ((oraculo))
//		w = false;
//	}
//}

//////////////////////////////////////////////////////
//           Renderização de Sprites                //
//////////////////////////////////////////////////////

void PPU::renderScanline(int scanline) {
	renderSprites(scanline);
}

void PPU::drawSpriteTile(Byte tileIndex, Byte x, Byte y, Byte attributes, int scanline) {
	// ! ! ! P L A C E H O L D E R ! ! !  para desenhar um tile do sprite ! ! !
	int spriteHeight = 8; // ou 16 se usar sprites altos (ver PPUCTRL)

	// Verifica a linha do sprite que está sendo desenhada
	int rowInTile = scanline - y;

	// Flipping vertical (bit 7 do atributo)
	if (attributes & 0x80) {
		rowInTile = spriteHeight - 1 - rowInTile;
	}

	// Cada tile tem 16 bytes (8 para plano baixo, 8 para alto)
	DWord patternBase = (ctrl.control & 0x08) ? 0x1000 : 0x0000;
	DWord baseAddress = patternBase + tileIndex * 16;
	Byte low = patternTable[baseAddress + rowInTile];
	Byte high = patternTable[baseAddress + rowInTile + 8];

	// Paleta (bits 0-1 do atributo)
	Byte palette = attributes & 0x03;

	// Flipping horizontal (bit 6)
	bool flipH = attributes & 0x40;

	for (int i = 0; i < 8; ++i) {
		int bit = flipH ? i : (7 - i);

		Byte bit0 = (low >> bit) & 1;
		Byte bit1 = (high >> bit) & 1;
		Byte colorIndex = (bit1 << 1) | bit0;

		if (colorIndex == 0) continue; // Cor 0 = transparente para sprites

		// Indice final da cor considerando paleta de sprites
		Byte paletteIndex = 0x10 + palette * 4 + colorIndex;

		Byte color = read(0x3F00 + (paletteIndex % 32)); // % 32 para evitar estouro

		// X do pixel
		int finalX = x + i;
		if (finalX >= 256 || scanline >= 240) continue; // Bounds check

		// Aqui você deve desenhar: substitua por sua função real de renderização
		putPixel(finalX, scanline, color);


	}
}

void PPU::putPixel(int x, int y, uint8_t colorIndex) {
	if (x < 0 || x >= 256 || y < 0 || y >= 240) return;

	Pixel cor = cores[colorIndex % 64];  // Garante que está dentro do range

	// RGBA8888 format: 0xRRGGBBAA
	uint32_t pixelValue = (0xFF << 24) | (cor.r << 16) | (cor.g << 8) | (cor.b);

	framebuffer[y * 256 + x] = pixelValue; // escreve no buffer SDL2
}


//////////////////////////////////////////////////////
//                Sprite 0-hit                      //
//////////////////////////////////////////////////////



void PPU::checkSpriteZeroHit(int scanline) {
	// Aqui vamos pegar as informações do sprite 0!!!
	Byte y = OAM[0];
	Byte tileIndex = OAM[1];
	Byte attributes = OAM[2];
	Byte x = OAM[3];

	// Verifica se o scanline atual atinge o sprite 0
	if (scanline >= y + 1 && scanline < y + 1 + 8) {
		int rowInTile = scanline - (y + 1);
		if (attributes & 0x80) {
			rowInTile = 7 - rowInTile; // flip vertical (QUE FOI TESTADO E FUNCIONA CORRETAMENTE EBAAAA)
		}

		DWord baseAddress = tileIndex * 16;
		Byte low = patternTable[baseAddress + rowInTile];
		Byte high = patternTable[baseAddress + rowInTile + 8];

		for (int i = 0; i < 8; i++) {
			int bit = (attributes & 0x40) ? i : (7 - i);

			Byte bit0 = (low >> bit) & 1;
			Byte bit1 = (high >> bit) & 1;

			if ((bit0 | bit1) == 0) continue; // pixel transparente no sprite

			// Coordenada do pixel no framebuffer
			int pixelX = x + i;
			int pixelY = scanline;

			// Verifica se background tem pixel não transparentes
			bool bgOpaque = backgroundBuffer[pixelY * 256 + pixelX] != 0;

			if (bgOpaque) {
				// CHECK SE DETECTOU UM HIT OMG NO WAY AY AY AY
				status.status |= 0x40; // seta bit 6
				return;
			}
		}
	}
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


// O que falta:
//
//	Scroll completo (principalmente pq o Sprite 0-hit depende disso)
//
//	Mirroring configurável
//
//	Suporte opcional a sprites 8x16
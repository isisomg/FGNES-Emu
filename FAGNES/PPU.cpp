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

bool PPUCTRL::isSpriteHeight16() const {
	// Retorna true se o bit 5 (0x20) estiver setado!!
	// ASSINADO: ISIS. >:(
	return (control & 0x20) != 0;
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
		// espelhamento
		if (paletteAddress == 0x10) { paletteAddress = 0x00; }
		else if (paletteAddress == 0x14) { paletteAddress = 0x04; }
		else if (paletteAddress == 0x18) { paletteAddress = 0x08; }
		else if (paletteAddress == 0x1C) { paletteAddress = 0x0C; }
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
		// Trata os espelhamentos de $3F10, $3F14, $3F18, $3F1C
		if (paletteAddress == 0x10) { // $3F10
			paletteAddress = 0x00;    // Espelha para $3F00
		}
		else if (paletteAddress == 0x14) { // $3F14
			paletteAddress = 0x04;    // Espelha para $3F04
		}
		else if (paletteAddress == 0x18) { // $3F18
			paletteAddress = 0x08;    // Espelha para $3F08
		}
		else if (paletteAddress == 0x1C) { // $3F1C
			paletteAddress = 0x0C;    // Espelha para $3F0C
		}

		paletteRAM[paletteAddress] = value;
	}
}


//////////////////////////////////////////////////////
//                    PPUMASK                       //					Falta implementar direito. Nao e dificil, logo ja faco
//////////////////////////////////////////////////////

void PPUMASK::write(Byte value) {
	// A CPU escreve no registrador $2001, e essa função atualiza as flags da máscara.
	// Cada bit do 'value' controla uma flag específica de renderização.

	grayscale = (value & 0x01) != 0;          // Bit 0: Escala de cinza
	showBackgroundLeft = (value & 0x02) != 0; // Bit 1: Mostrar background nos 8 pixels da esquerda
	showSpritesLeft = (value & 0x04) != 0;    // Bit 2: Mostrar sprites nos 8 pixels da esquerda
	showBackground = (value & 0x08) != 0;     // Bit 3: Mostrar background
	showSprites = (value & 0x10) != 0;        // Bit 4: Mostrar sprites
	emphasizeRed = (value & 0x20) != 0;       // Bit 5: Enfatizar vermelho
	emphasizeGreen = (value & 0x40) != 0;     // Bit 6: Enfatizar verde
	emphasizeBlue = (value & 0x80) != 0;      // Bit 7: Enfatizar azul
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

//MirroringSelect mirroringselect = MirroringSelect::Horizontal; // DEFINIDO PELO CARTUCHO AGORA

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

// DWord PPU::mirrorAddress(DWord address) {
//	address = (address - 0x2000) % 0x1000;
//	return address % 0x800;
//}



//////////////////////////////////////////////////////
//            VBLANK (FEITO) & STEP                 //										uhul!
//////////////////////////////////////////////////////

bool nmiRequested = false;

void (*nmiCallback)() = nullptr;

void PPU::step() {
	dot++;
	if (dot > 340) {
		dot = 0;
		scanline++;
		if (scanline > 261) {
			scanline = 0;
		}
	}

	bool renderingEnabled = mask.showBackground || mask.showSprites;

	// Scanline de pré-renderização (261)
	if (scanline == 261 && dot == 1) {
		status.setVBlank(false);
		status.status &= ~0x40; // Limpa o flag de Sprite 0 Hit
		status.status &= ~0x20; // Limpa o flag de Sprite Overflow
	}

	// Scanline visiveis de 0 á 240
	if (scanline >= 0 && scanline < 240) {
		if (renderingEnabled) {

			// No dot 256 a PPU conseguiu pegar todo o background e os sprites da scanline atual!!!!!
		
			// Aqui vamos fazer a renderizacao dela e coisar o Y para a prox scanline!
			
			if (dot == 256) {
				renderBackgroundScanline(scanline);
				renderSprites(scanline);
				checkSpriteZeroHit(scanline);

				incrementY(); // Aqui tamo preparando o scroll vertical pra prox scanline
			}
			// No dot 257, a PPU vai restaurar a posição horizontal de 't' para 'v'.
			// 
			// Isso vai resetar a posicao horizontal pro começo da prox scanline!!!
			else if (dot == 257) {
				v = (v & ~0x041F) | (t & 0x041F);
			}
		}
	}
	// Agora durante a scanline 241 e no dot 1, começa o V-Blank!!!
	else if (scanline == 241 && dot == 1) {
		status.setVBlank(true);
		if (ctrl.isNMIEnabled()) {
			nmiRequested = true;
		}
	}
	// Scanline de Pre Renderizaxcao
	else if (scanline == 261) {
		// No dot 1, todas as flags de renderizacao setadas vao ser limpas!!
		if (dot == 1) {
			status.setVBlank(false);
			status.status &= ~0x40; // Limpa a tag de sprite 0 hit
			status.status &= ~0x20; // Limpa o flag de Sprite Overflow
		}

		if (renderingEnabled) {
			// Durante os dots 280-304, a PPU copia repetinamente repetitidamente repetitivemamente (??? SEILA CARA) os bits verticais de t pra v.
			// Isso carrega a posicao do scroll vertical pro prox frame!
			if (dot >= 280 && dot <= 304) {
				v = (v & ~0x7BE0) | (t & 0x7BE0);
			}
		}
	}
}


void PPU::renderBackgroundScanline(int scanline) {
	if (!mask.showBackground) {
		return;
	}

	// Usa uma cópia temporária de 'v' para não alterar o estado real durante a renderização
	DWord temp_v = v;
	DWord patternBase = (ctrl.control & 0x10) ? 0x1000 : 0x0000;

	// Desenha 33 tiles para cobrir os 256 pixels + rolagem fina
	for (int tile_n = 0; tile_n < 33; ++tile_n) {

		// Busca o tile, atributo e dados do padrão
		DWord tileAddr = 0x2000 | (temp_v & 0x0FFF);
		Byte tileIndex = read(tileAddr);

		DWord attrAddr = 0x23C0 | (temp_v & 0x0C00) | ((temp_v >> 4) & 0x38) | ((temp_v >> 2) & 0x07);
		Byte attrByte = read(attrAddr);
		int shift = ((temp_v >> 4) & 4) | (temp_v & 2);
		Byte paletteBits = (attrByte >> shift) & 0x03;

		DWord fineY = (temp_v >> 12) & 0x07;
		DWord patternAddr = patternBase + (DWord)tileIndex * 16 + fineY;
		Byte lowPlane = read(patternAddr);
		Byte highPlane = read(patternAddr + 8);

		// Renderiza os 8 pixels do tile atual
		for (int pixel = 0; pixel < 8; ++pixel) {
			int screenX = tile_n * 8 + pixel - x;
			if (screenX < 0) screenX += 256;
			if (screenX >= 256) continue;

			Byte bit0 = (lowPlane >> (7 - pixel)) & 1;
			Byte bit1 = (highPlane >> (7 - pixel)) & 1;
			Byte colorIndex = (bit1 << 1) | bit0;

			Byte finalColorIndex = 0;
			if (colorIndex != 0) {
				Byte paletteIndex = paletteBits * 4 + colorIndex;
				finalColorIndex = paletteRAM[paletteIndex & 0x1F];
			}
			else {
				finalColorIndex = paletteRAM[0]; // Cor de fundo universal (Que é o cinza maldito que tava printando quando eu tava fazendo ESSA DESGRAÇA INFERNAL)
			}

			Pixel cor = cores[finalColorIndex % 64];
			framebuffer[scanline * 256 + screenX] = (0xFF << 24) | (cor.r << 16) | (cor.g << 8) | cor.b;
			backgroundBuffer[scanline * 256 + screenX] = (colorIndex != 0);
		}

		// Após processar um tile, incrementa o coarse X no nosso v temp
		if ((temp_v & 0x001F) == 31) {
			temp_v &= ~0x001F;
			temp_v ^= 0x0400;
		}
		else {
			temp_v++;
		}
	}
}

// VAI VERIFICAR SE PRECISA DO NMI
bool PPU::isNMIRequested() {
	bool result = nmiRequested;
	nmiRequested = false;
	return result;
}

//////////////////////////////////////////////////////
//        Scroller MEIO que funcionando?            //   (VERTICAL TOTALMENTE OK, HORIZONTAL DANDO ARTEFATO MAS NAO FICA MAIS PRESO EM LOOP)
//////////////////////////////////////////////////////


void PPU::incrementX() {
	// If coarse X is maxed out (31), wrap to 0 and switch horizontal nametable
	if ((v & 0x001F) == 31) {
		v &= ~0x001F; // Coarse X = 0
		v ^= 0x0400;  // Switch horizontal nametable
	}
	else {
		v += 1; // Increment coarse X
	}
}

void PPU::incrementY() {
	// se fineY e menor que 7, incrementa fine Y
	if ((v & 0x7000) != 0x7000) {
		v += 0x1000;
	}
	else {
		// Se fine Y e 7, ele da overflow para 0 e vai acabar incrementando pro nosso coarse Y
		v &= ~0x7000;
		int y = (v & 0x03E0) >> 5; // Pega o coarse Y
		if (y == 29) {
			y = 0;
			v ^= 0x0800; // Se coarse Y for 29, ele vai dar overflow para 0 e mudar a nametable vertical
		}
		else if (y == 31) {
			y = 0; // Coarse Y vai de 31 para 0
		}
		else {
			y += 1; // Incrementa o coarse Y
		}
		// Atualiza o coarse Y no v
		v = (v & ~0x03E0) | (y << 5);
	}
}


//////////////////////////////////////////////////////
//           Renderização de Sprites                //
//////////////////////////////////////////////////////

void PPU::renderScanline(int scanline) {
	renderSprites(scanline);
}

void PPU::drawSpriteTile(Byte tileIndex, Byte x, Byte y, Byte attributes, int scanline) {
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

	// Verifica o bit de prioridade de sprite que eh o bit tinco
	bool backgroundPriority = (attributes & 0x20) != 0;

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

		// Se o pixel do fundo não for transparente e o sprite estiver atras do fundo, ele nao vai ser desenhado. (O mario por exemplo nao vai aparecer atras do cano e tals)
		if (backgroundBuffer[scanline * 256 + finalX] && backgroundPriority) {
			continue;
		}

		// Aqui você deve desenhar: substitua por sua função real de renderização
		putPixel(finalX, scanline, color);

	}
}

void PPU::putPixel(int x, int y, uint8_t colorIndex) {
	if (x < 0 || x >= 256 || y < 0 || y >= 240) return;

	Pixel cor = cores[colorIndex % 64];  // Garante que está dentro do range

	// RGBA8888 format: 0xRRGGBBAA
	uint32_t pixelValue = (0xFF << 24) | (cor.r << 16) | (cor.g << 8) | cor.b;

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

	// O hit so pode acontecer se a renderização de background e sprites estiver habilitada
	if (!mask.showBackground || !mask.showSprites) {
		return;
	}

	int spriteHeight = ctrl.isSpriteHeight16() ? 16 : 8;

	// Aqui vamos verificar se a scanline atual atinge a faixa vertical do sprite 0

	if (scanline >= y + 1 && scanline < y + 1 + spriteHeight) { // 8 ou 16 pixeis de altura
		DWord patternAddr;
		int rowInTile = scanline - (y + 1);

		if (spriteHeight == 16) {		// No caso de sprites de 16 pixels de altura
			bool flipV = (attributes & 0x80) != 0;
			if (flipV) rowInTile = 15 - rowInTile;		// Inverte a linha se o sprite estiver flipado verticalmente

			Byte targetTile;
			if (rowInTile < 8) {
				targetTile = tileIndex & 0xFE;		// Mantém o tile original (0-7)
			}
			else {
				targetTile = (tileIndex & 0xFE) + 1;
				rowInTile -= 8;		// Ajusta a linha para o segundo tile (8-15)
			}
			DWord patternTableBase = (tileIndex & 0x01) ? 0x1000 : 0x0000;
			patternAddr = patternTableBase + targetTile * 16 + rowInTile;		// Calcula o endereço do padrao do tile
		}
		else {		// No caso de sprites de 8 pixels de altura
			if (attributes & 0x80) rowInTile = 7 - rowInTile;
			DWord patternTableBase = (ctrl.control & 0x08) ? 0x1000 : 0x0000;
			patternAddr = patternTableBase + tileIndex * 16 + rowInTile;		// Calcula o endereço do padrao do tile
		}

		Byte lowPlane = read(patternAddr);
		Byte highPlane = read(patternAddr + 8);

		for (int i = 0; i < 8; ++i) {			// Verifica os 8 pixels do tile
			int finalX = x + i;
			if (finalX >= 256) continue;

			// Verificacoes de bordas!!!!
			if (finalX == 255) continue;		// Evita overflow no buffer de background
			if (finalX < 8 && (!mask.showBackgroundLeft || !mask.showSpritesLeft)) continue;	// Se os 8 pixels da esquerda nao devem ser mostrado ele pula

			bool flipH = (attributes & 0x40) != 0;		// Verifica se o sprite está flipado horizontalmente
			int bit = flipH ? i : (7 - i);		// Inverte o bit se estiver flipado horizontalmente

			Byte bit0 = (lowPlane >> bit) & 1;
			Byte bit1 = (highPlane >> bit) & 1;
			Byte colorIndex = (bit1 << 1) | bit0;

			if (colorIndex != 0 && backgroundBuffer[scanline * 256 + finalX]) {
				status.status |= 0x40;
				return;
			}
		}
	}
}

void PPU::renderSprites(int scanline) {
	// Informações em: https://www.nesdev.org/wiki/PPU_OAM


	// Sprite Height depende de config, aqui 8 como padrão

	int spriteHeight = ctrl.isSpriteHeight16() ? 16 : 8; // Aqui mudei pra aceitar sprites de 16 pixels de altura!!
	int spritesOnThisLine = 0;

	for (int i = 0; i < 64; ++i) {
		Byte y = OAM[i * 4 + 0];
		Byte tileIndex = OAM[i * 4 + 1];
		Byte attributes = OAM[i * 4 + 2];
		Byte x = OAM[i * 4 + 3];

		// Ignora sprites que estão fora da tela (y > 239)
		if (y >= 0xEF) continue;

		if (scanline >= y + 1 && scanline < y + 1 + spriteHeight) {
			if (spritesOnThisLine >= 8) break;
			drawSpriteTile(tileIndex, x, y + 1, attributes, scanline);
			spritesOnThisLine++;

			DWord patternAddr;
			int rowInTile = scanline - (y + 1);

			// Lógica para sprites 8x16
			if (spriteHeight == 16) {
				bool flipV = (attributes & 0x80) != 0;

				// Se houver flip vertical, a ordem dos tiles é invertida
				if (flipV) {
					rowInTile = 15 - rowInTile;
				}

				// Determina qual tile (superior ou inferior) usar
				Byte targetTile;
				if (rowInTile < 8) { // Scanline está na metade superior do sprite
					targetTile = tileIndex & 0xFE; // Usa o primeiro tile do par
				}
				else { // Scanline está na metade inferior
					targetTile = (tileIndex & 0xFE) + 1; // Usa o segundo tile
					rowInTile -= 8; // Ajusta a linha para ser relativa ao tile inferior
				}
				
				// O bit 0 do índice original do tile seleciona a pattern table
				
				DWord patternTableBase = (tileIndex & 0x01) ? 0x1000 : 0x0000;
				patternAddr = patternTableBase + targetTile * 16 + rowInTile;
			}
			
			// Lógica original para sprites 8x8
			else {
				if (attributes & 0x80) { // Flip Vertical
					rowInTile = 7 - rowInTile;
				}
				// A pattern table é selecionada pelo PPUCTRL bit 3
				DWord patternTableBase = (ctrl.control & 0x08) ? 0x1000 : 0x0000;
				patternAddr = patternTableBase + tileIndex * 16 + rowInTile;
			}
			
			// Aqui vamos renderizar o tile
			
			Byte lowPlane = read(patternAddr);
			Byte highPlane = read(patternAddr + 8);
			Byte palette = attributes & 0x03;
			bool flipH = (attributes & 0x40) != 0;
			bool backgroundPriority = (attributes & 0x20) != 0;

			for (int pixel = 0; pixel < 8; ++pixel) {
				int finalX = x + pixel;
				if (finalX >= 256) continue;

				int bit = flipH ? pixel : (7 - pixel);
				Byte bit0 = (lowPlane >> bit) & 1;
				Byte bit1 = (highPlane >> bit) & 1;
				Byte colorIndex = (bit1 << 1) | bit0;

				if (colorIndex == 0) continue;

				if (backgroundPriority && backgroundBuffer[scanline * 256 + finalX]) {
					continue;
				}

				Byte paletteIndex = 0x10 + palette * 4 + colorIndex;
				Byte finalColor = read(0x3F00 + paletteIndex);
				putPixel(finalX, scanline, finalColor);
			}
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

//////////////////////////////////////////////////////
//                Interface com CPU                 //
//////////////////////////////////////////////////////

void PPU::cpuWrite(DWord addr, Byte data) {
	switch (addr & 0x0007) {
	case 0x0:	// $2000 - PPUCTRL
		ctrl.write(data);
		// Atualiza os bits da nametable no endereço temporário 't'
		t = (t & 0xF3FF) | ((DWord)(data & 0x03) << 10);
		break;
	case 0x1:	// $2001 - PPUMASK
		mask.write(data);
		break;
	case 0x3:	// $2003 - OAMADDR
		oamAddress = data;
		break;
	case 0x4:	// $2004 - OAMDATA
		OAM[oamAddress] = data;
		oamAddress++;
		break;
	case 0x5:   // $2005 - PPUSCROLL (ESSA PARTE ESTAVA FALTANDO)
		if (!w) { // Primeira escrita
			t = (t & 0xFFE0) | (data >> 3); // Coarse X scroll
			x = data & 0x07;                // Fine X scroll
			w = true;
		}
		else {  // Segunda escrita
			t = (t & 0x8C1F) | ((DWord)(data & 0xF8) << 2); // Coarse Y scroll
			t = (t & 0x0FFF) | ((DWord)(data & 0x07) << 12); // Fine Y scroll
			w = false;
		}
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
		w = false; // Leitura de PPUSTATUS reseta o latch 'w'
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

void PPU::writeToPPUADDR(Byte value) {
	if (!w) { // Primeira escrita
		// Garante que os bits 14 e 15 de 't' (formados por esta escrita) sejam 0
		t = (t & 0x00FF) | (((DWord)value & 0x3F) << 8);
		w = true;
	}
	else {  // Segunda escrita
		t = (t & 0xFF00) | value;
		v = t; // Na segunda escrita, t é copiado para v
		w = false;
	}
}


Byte PPU::readFromPPUData() {
	Byte value = read(v);

	if (v < 0x3F00) {					// Se v for menor que 0x3F00, significa que estamos lendo da VRAM normal
		Byte buffered = ppuDataBuffer;
		ppuDataBuffer = value;
		value = buffered;
	}
	else {								// Se v for maior ou igual a 0x3F00, significa que estamos lendo da paleta!!!!!!!!!!!!!
		ppuDataBuffer = read(v - 0x1000);
	}

	// Incrementa v de acordo com o bit 2 do PPUCTRL (bit 2 = 0x04)
	v += (ctrl.control & 0x04) ? 32 : 1;
	return value;
}

// Essa função escreve no PPUData, incrementando v de acordo com o bit 2 do PPUCTRL (bit 2 = 0x04)
void PPU::writeToPPUData(Byte value) {
	write(v, value);
	v += (ctrl.control & 0x04) ? 32 : 1;
}

// Oq da pra melhorar? SOMENTE sprite overflow, maioria dos jogos agora ja se torna jogaveis (mapper 0 - testado) eba :)

// Jogos nao jogaveis sabe-se la pq:

// Paperboy (mapper 0) - Fica todo maluco doido morto
// Kung-Fu (mapper 0) - Vc da UM soco e ele MORRE e CRASHA INTEIRO.
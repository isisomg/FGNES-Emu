#include <cstdint>
#include "Tipos.h"

struct PPUCTRL {
	Byte control = 0;
	// Haha inteiro de 8 bits que é de 0 a 255, o NES usa registradores de 8 bits ent é isso que vamos usar!!
	// Quando a CPU escrever no registrador $2000, o PPUCTRL é atualizado, vamos guardar esse valor na variavel control ta??? EU TO ESCREVENDO ISSO PQ EU ACHO Q É IMPORTANTE ENTENDER COMO FUNCIONA.
	// ASSINADO: ISIS. >:(
	void write(Byte value) {
		control = value;
	}

	// Pelo oq entendi, na cpu vai terq chamar essa fução, e ativar o NMI lá. 
	// ASSINADO: ISIS. >:(
	bool isNMIEnabled() const {
		return (control & 0x80) != 0;
	}

	// Esse aqui PELO OQ ENTENDI, nao precisa de comunicação com a CPU, é só um getter mesmo.
	// ASSINADO: ISIS. >:(
	bool isMasterSlave() const {
		return (control & 0x40) != 0;
	}

	// Esse aqui também é só um getter, mas ele vai retornar o valor do bit 3 do control.
	// ASSINADO: ISIS. >:(
	Byte getNameTableAddr() const {
		return control & 0x03;
	}
};

// O PPUSTATUS e um registrador de status do PPU, que armazena informacoes sobre o estado atual do PPU.
// ASSINADO: ISIS > :(
struct PPUSTATUS {
	Byte status = 0;

	Byte read() {
		Byte result = status;
		status &= ~0x80; // Aqui ele meio que limpou o bit 7, que é associado ao VBlank, depois ele vai retornar o status.	ASSINADO: ISIS >:(
		return result;
	}

	// Aqui ele vai setar o bit 7 do status, que é o VBlank.
	// ASSINADO: ISIS >:(
	void setVBlank(bool value) {
		if (value) {				//	Se o valor for verdadeiro, ele vai setar o bit 7 do status.
			status |= 0x80;
		}
		else {						// Se o valor for falso, ele vai limpar o bit 7 do status.
			status &= ~0x80;
		}
	}
};

struct PPU {
	PPUCTRL ctrl;
	PPUSTATUS status;

	//////////////////////////////////////////////////////////////////////////	  VRAM	  ///////////////////////////////////////////////////////////////////////////////

	// Aqui vamos implementar a VRAM (video ram wowowow!!) Referencia: https://www.nesdev.org/wiki/PPU_memory_map
	// ASSINADO: ISIS. >:(

	// Se vc ler o link, (EU LI EU JURO), o NES tem apenas 2kb de RAM dedicados pra PPU, mas a PPU endereça até 16kb!!!!
	// Existe uma diferença entre espaço de endereçamento e memória física real.
	// Os 2kb de RAM sao utilizados principalmente pra armazenar os mapas de nametables de backgrounds.
	// MAS os outros 14kb sao utilizados para enxergar endereços de tiles graficos, mirroring que deve ser traduzido como espelhar e paleta de cores!!
	// ASSINADO: ISIS >:(

	// Entao aqui vamos reservar 2kb reais pra nametables
	Byte nametableVRAM[2048] = { 0 };
	Byte patternTable[0x2000] = { 0 };
	Byte paletteRAM[32] = { 0 };

	Byte read(uint16_t address) {
		address &= 0x3FFF; // Faixa válida da PPU

		if (address >= 0x2000 && address <= 0x3EFF) {
			// Acessando nametables e espelhos
			uint16_t mirroredAddress = mirrorAddress(address);
			return nametableVRAM[mirroredAddress];
		}

		if (address < 0x2000) {
			return patternTable[address];
		}

		if (address >= 0x3F00 && address <= 0x3FFF) {
			uint16_t paletteAddress = (address - 0x3F00) % 32;
			return paletteRAM[paletteAddress];
		}
		return 0x00;
	}

	void write(uint16_t address, Byte value) {
		address &= 0x3FFF;

		if (address < 0x2000) {
			patternTable[address] = value;
		}
		else if (address >= 0x2000 && address <= 0x3EFF) {
			uint16_t mirroredAddress = mirrorAddress(address);
			nametableVRAM[mirroredAddress] = value;
		}
		else if (address >= 0x3F00 && address <= 0x3FFF) {
			uint16_t paletteAddress = (address - 0x3F00) % 32;
			paletteRAM[paletteAddress] = value;
		}
	}

	uint16_t mirrorAddress(uint16_t address) {
		// Endereços de nametable vão de 0x2000 a 0x2FFF
		// Com espelhos até 0x3EFF
		address = (address - 0x2000) % 0x1000;

		// Aplica espelhamento
		// Isso depende do cartucho!!!!!!!!!!!!!!! mas vamos supor vertical:
		return address % 0x800;
	}

	////////////////////////////////////////////////////////////////////////////				Scan-lines				  ///////////////////////////////////////////////////////////////////////////////

	// Contadores de varredura (scanlines) e pontos (dots)
	int scanline = 0; // De 0 a 261 (262 scanlines por frame)
	int dot = 0;      // De 0 a 340 (341 pontos por linha)

	// A funcao step que eu fiz renderiza uma linha de sprites
	// Eu vs matematica basica
	void step() {
		dot++;

		if (dot > 340) {
			dot = 0;
			scanline++;

			//O QUE TA ACONTECENDO AQUI??!! Eu, Isis Calabresi explico! O NES roda um frame completo com 262 scanlines
			// De 0 a 239 ocorre a renderização, de 240 ocorre uma linha de descanco, de 241 a 260 ocorre o VBlank, e em 261 ocorre uma scanline especial de pre-render.

			if (scanline > 261) {
				scanline = 0;
				// Isso significa que um frame ACABOU.
			}
		}

		if (scanline == 241 && dot == 1) {
			status.setVBlank(true);
			if (ctrl.isNMIEnabled()) {
				// NMI, ou seja, espera instrucao da CPU.
			}
		}

		if (scanline == 261 && dot == 1) {
			status.setVBlank(false);
		}

		if (scanline < 240) {
			// Linha de descanço como eu ACABEI de explicar. >:d
			renderScanline(scanline);
		}
	}

	void renderScanline(int scanline) {
		renderSprites(scanline);
	}

	void drawSpriteTile(Byte tileIndex, Byte x, Byte y, Byte attributes, int scanline) {
		// Literalmente SOMENTE UM PLACEHOLDER pra evitar erro de compilacao e pra nao explodir tudo
	}

	void renderSprites(int scanline) {
		int spriteHeight = 8; // futuro: 8 ou 16
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

	////////////////////////////////////////////////////////////////////////////	  Object Attribute Memory (OAM)		  ///////////////////////////////////////////////////////////////////////////////

	// Mais informações em https://www.nesdev.org/wiki/PPU_OAM

	// Como escrito na biblia do nesdev.org, a OAM e uma memoria que contem uma lista de display de ate 64 sprites. Cada sprite ocupando 4 bytes.
	// 64 x 4 = 256 bytes de OAM, que é o que vamos implementar aqui.
	// APARTIR DAQUI EU VOU PARAR DE ASSINAR PQ JA TA FICANDO SEM GRAÇA E DEU PRA ENTENDER NE

	Byte OAM[256] = { 0 };
	Byte oamAddress = 0x00;

	// ACREDITO MTO MTO MTO MTO MTO QUE ISSO TA ERRADO, DE VERDADE, EU ODEIO OBJECT ATTRIBUTE MEMORY HAHAHAHAHA
	// Depois temq implementar isso com a CPU eu tenho certeza.
	void doOAMDMA(const Byte* cpuMemoryPage) {
		for (int i = 0; i < 256; ++i) {
			OAM[i] = cpuMemoryPage[i];
		}
	}

	// Registradores de controle de endereço
	uint16_t ppuAddress = 0x0000;
	bool addressLatch = false;

	// Aqui a gente vai escrever no registrador de endereço do PPU, que é o $2006.
	void writeToPPUADDR(Byte value);

	// Interface com a CPU
	void cpuWrite(uint16_t addr, Byte data) {
		switch (addr & 0x0007) {
		case 0x0: // $2000 - PPUCTRL
			ctrl.write(data);
			break;

		case 0x3: // $2003 - OAMADDR
			// MEIO IMPORTANTE TA: OAMDDR SO vai escrever, nao ler, entao nao vamos colocar ele na cpu write
			oamAddress = data;
			break;

		case 0x4: // $2004 - OAMDATA
			OAM[oamAddress] = data;
			oamAddress++;
			break;

		case 0x6: // $2006 - PPUADDR
			writeToPPUADDR(data);
			break;

		default:
			// Mais registradores que eu infelizmente terei de implementar.
			break;
		}
	}

	Byte cpuRead(uint16_t addr) {
		Byte data = 0x00;
		switch (addr & 0x0007) {
		case 0x2: // $2002 - PPUSTATUS
			data = status.read();
			addressLatch = false;
			break;

		case 0x4: // $2004 - OAMDATA
			data = OAM[oamAddress];
			break;

		default:
			// Mais registradores que eu infelizmente nao implementei, mas que eu vou implementar, EU JURO, EU JURO MESMO!!!!!!!!
			break;
		}
		return data;
	}

	// NMI
	// Callback que a CPU vai registrar pra ser chamada quando o PPU quiser gerar um NMI
	void (*nmiCallback)() = nullptr;
};

//////////////////////////////////////////////////////////////////////////////////////////		FALTA:  	 ////////////////////////////////////////////////////////////////////////////////////////////////////

//				COISAS QUE FALTAM E QUE PRECISAM PRO FAGNES SER FUNCIONAVEL:
// 
//	Coiso							Importancia						Descricao
// Escrita\leitura em $2007				Alta		Fundamental para interagir com VRAM via CPU.
// Pattern tables($0000 - $1FFF)		Alta		Sem isso nao da pra renderizar tiles.
// Paletas($3F00 - $3FFF)				Alta		Precisa para cor real na tela.
// Renderização real do background		Alta		Precisa buscar tiles, atributos e desenhar.
// Sprites(OAM) na tela					Media		Mas necessario para jogos funcionarem.
// Scroll($2005, $2006)					Media		Fundos moveis exigem isso.

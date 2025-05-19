#include <cstdint>
#include "Tipos.h"

struct PPUCTRL {
	Byte control = 0;
	void write(Byte value);
	bool isNMIEnabled() const;
	bool isMasterSlave() const;
	Byte getNameTableAddr() const;
};

struct PPUSTATUS {
	Byte status = 0;
	Byte read();
	void setVBlank(bool value);
};

struct PPU {
	PPUCTRL ctrl;
	PPUSTATUS status;

	//////////////////////////////////////////////////////
	//                     VRAM                         //
	//////////////////////////////////////////////////////

	// https://www.nesdev.org/wiki/PPU_memory_map
	
	Byte nametableVRAM[2048] = { 0 };
	Byte patternTable[0x2000] = { 0 };
	Byte paletteRAM[32] = { 0 };

	Byte read(DWord address);

	void write(DWord address, Byte value);

	DWord mirrorAddress(DWord address);


	//////////////////////////////////////////////////////
	//                  Scan-lines                      //
	//////////////////////////////////////////////////////
	
	int scanline = 0;
	int dot = 0; 


	void step();
	void renderScanline(int scanline);
	void drawSpriteTile(Byte tileIndex, Byte x, Byte y, Byte attributes, int scanline);
	void renderSprites(int scanline);
	//////////////////////////////////////////////////////
	//                      OAM                         //
	//////////////////////////////////////////////////////

	// Como escrito na biblia do nesdev.org, a OAM e uma memoria que contem uma lista de display de ate 64 sprites. Cada sprite ocupando 4 bytes.

	Byte OAM[256] = { 0 };		// 64 x 4 = 256 bytes de OAM, que é o que vamos implementar aqui.
	Byte oamAddress = 0x00;		// Endereço de OAM que estamos escrevendo.

	void doOAMDMA(const Byte* cpuMemoryPage);

	// Registradores de controle de endereço
	DWord ppuAddress = 0x0000;
	bool addressLatch = false;

	void writeToPPUADDR(Byte value);

	// Interface com a CPU
	void cpuWrite(DWord addr, Byte data);

	Byte cpuRead(DWord addr);

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

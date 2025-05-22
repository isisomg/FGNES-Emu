#include <cstdint>
#include "Tipos.h"
#include "Cores.h"
#include <vector>

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
	bool isNMIRequested();
	PPUCTRL ctrl;
	PPUSTATUS status;

	// Aqui PRECISA MESMO MESMO EMESMO de um background buffer paralelo, com informações de opacidade do background para que o sprite zero hit funcione corretamente.
	Byte backgroundBuffer[256 * 240]; // 1 byte por pixel, só pra saber se é transparente ou não

	//////////////////////////////////////////////////////
	//                     VRAM                         //
	//////////////////////////////////////////////////////

	// https://www.nesdev.org/wiki/PPU_memory_map

	Byte nametableVRAM[2048] = { 0 };
	Byte patternTable[0x2000] = { 0 };
	Byte paletteRAM[32] = { 0 };
	Byte ppuDataBuffer = 0x00; // Buffer de leitura da VRAM

	Byte read(DWord address);
	void write(DWord address, Byte value);

	void carregarCHR(const std::vector<Byte>& chrData);
	void putPixel(int x, int y, uint8_t colorIndex); // escrever buffer SDL.
	uint32_t framebuffer[256 * 240] = {0}; // buffer para o SDL2 desenhar o frame.

	// $2007 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	Byte readFromPPUData();
	void writeToPPUData(Byte value);

	DWord mirrorAddress(DWord address);


	//////////////////////////////////////////////////////
	//                  Scan-lines                      //
	//////////////////////////////////////////////////////

	int scanline = 0;
	int dot = 0;



	void step();
	void renderScanline(int scanline);
	void renderBackgroundScanline(int scanline);
	void renderSprites(int scanline);
	void drawSpriteTile(Byte tileIndex, Byte x, Byte y, Byte attributes, int scanline);
	void checkSpriteZeroHit(int scanline);  // Aqui eu isis calabresi acabei de adicionar o sprite zero hit




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

//////////////////////////////////////////////////////
//			        MIRRORING!!						//									EBA EBA EBA CONSEGUI
//////////////////////////////////////////////////////
// https://www.nesdev.org/wiki/Mirroring

enum struct MirroringSelect {
	Horizontal,
	Vertical
};

extern MirroringSelect mirroringselect; //tive que fazer esse extern pq tava dando um problema com a APU.

//////////////////////////////////////////////////////////////////////////////////////////		FALTA:  	 ////////////////////////////////////////////////////////////////////////////////////////////////////

//	O que falta?
//
//	Scroll completo
//
//	Mirroring configurável
//
//	Suporte opcional a sprites 8x16
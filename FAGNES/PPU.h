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
	//void setSprite0Hit(bool value);			 Talvez eu tenha que fazer um rework disso, pra funcionar mais eficientemente. Por isso ta aqui como comentário.
	//void setSpriteOverflow(bool value);		 Disso tb
	void reset();
};

struct PPUMASK {
	bool grayscale = false; // 0x01
	bool showBackgroundLeft = false; // 0x02
	bool showSpritesLeft = false; // 0x04
	bool showBackground = false; // 0x08
	bool showSprites = false; // 0x10
	bool emphasizeRed = false; // 0x20
	bool emphasizeGreen = false; // 0x40
	bool emphasizeBlue = false; // 0x80

	void write(Byte value);
};

struct PPU {
	bool isNMIRequested();
	PPUCTRL ctrl;
	PPUMASK mask;
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
	uint32_t framebuffer[256 * 240] = { 0 }; // buffer para o SDL2 desenhar o frame.

	// $2007 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	Byte readFromPPUData();
	void writeToPPUData(Byte value);

	DWord mirrorAddress(DWord address);

	//////////////////////////////////////////////////////
		//                   Scroller & VRAM Address        //
		//////////////////////////////////////////////////////

	DWord v = 0;   // Endereço VRAM atual (15 bits)
	DWord t = 0;   // Endereço VRAM temporário (15 bits)
	Byte x = 0;    // Rolagem Fina X (3 bits)
	bool w = false; // Latch de escrita (para $2005 e $2006)

	// Métodos de incremento
	void incrementX();
	void incrementY();


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
	void checkSpriteZeroHit(int scanline);


	//////////////////////////////////////////////////////
	//                      OAM                         //
	//////////////////////////////////////////////////////

	Byte OAM[256] = { 0 };
	Byte oamAddress = 0x00;

	void doOAMDMA(const Byte* cpuMemoryPage);

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
	Vertical,
	FourScreen // NAO IMPLEMENTTADO MEDONHO
};

extern MirroringSelect mirroringselect; //tive que fazer esse extern pq tava dando um problema com a APU.
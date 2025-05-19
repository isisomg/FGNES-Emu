#include <cstdint>
#include "Tipos.h"

struct PPUCTRL {
	Byte control = 0;
	// Haha inteiro de 8 bits que é de 0 a 255, o NES usa registradores de 8 bits ent é isso que vamos usar!!
	// Quando a CPU escrever no registrador $2000, o PPUCTRL é atualizado, vamos guardar esse valor na variavel control ta??? EU TO ESCREVENDO ISSO PQ EU ACHO Q É IMPORTANTE ENTENDER COMO FUNCIONA.
	// ASSINADO: ISIS. >:(
	void write(Byte value);

	// Pelo oq entendi, na cpu vai terq chamar essa fução, e ativar o NMI lá. 
	// ASSINADO: ISIS. >:(
	bool isNMIEnabled() const;

	// Esse aqui PELO OQ ENTENDI, nao precisa de comunicação com a CPU, é só um getter mesmo.
	// ASSINADO: ISIS. >:(
	bool isMasterSlave() const;

	// Esse aqui também é só um getter, mas ele vai retornar o valor do bit 3 do control.
	// ASSINADO: ISIS. >:(
	Byte getNameTableAddr() const;
};

// O PPUSTATUS e um registrador de status do PPU, que armazena informacoes sobre o estado atual do PPU.
// ASSINADO: ISIS > :(
struct PPUSTATUS {
	Byte status = 0;

	Byte read();

	// Aqui ele vai setar o bit 7 do status, que é o VBlank.
	// ASSINADO: ISIS >:(
	void setVBlank(bool value);
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

	Byte read(uint16_t address);

	void write(uint16_t address, Byte value);

	uint16_t mirrorAddress(uint16_t address);

	////////////////////////////////////////////////////////////////////////////				Scan-lines				  ///////////////////////////////////////////////////////////////////////////////

	// Contadores de varredura (scanlines) e pontos (dots)
	int scanline = 0; // De 0 a 261 (262 scanlines por frame)
	int dot = 0;      // De 0 a 340 (341 pontos por linha)

	// A funcao step que eu fiz renderiza uma linha de sprites
	// Eu vs matematica basica
	void step();

	void renderScanline(int scanline);

	void drawSpriteTile(Byte tileIndex, Byte x, Byte y, Byte attributes, int scanline);

	void renderSprites(int scanline);

	////////////////////////////////////////////////////////////////////////////	  Object Attribute Memory (OAM)		  ///////////////////////////////////////////////////////////////////////////////

	// Mais informações em https://www.nesdev.org/wiki/PPU_OAM

	// Como escrito na biblia do nesdev.org, a OAM e uma memoria que contem uma lista de display de ate 64 sprites. Cada sprite ocupando 4 bytes.
	// 64 x 4 = 256 bytes de OAM, que é o que vamos implementar aqui.
	// APARTIR DAQUI EU VOU PARAR DE ASSINAR PQ JA TA FICANDO SEM GRAÇA E DEU PRA ENTENDER NE

	Byte OAM[256] = { 0 };
	Byte oamAddress = 0x00;

	// ACREDITO MTO MTO MTO MTO MTO QUE ISSO TA ERRADO, DE VERDADE, EU ODEIO OBJECT ATTRIBUTE MEMORY HAHAHAHAHA
	// Depois temq implementar isso com a CPU eu tenho certeza.
	void doOAMDMA(const Byte* cpuMemoryPage);

	// Registradores de controle de endereço
	uint16_t ppuAddress = 0x0000;
	bool addressLatch = false;

	// Aqui a gente vai escrever no registrador de endereço do PPU, que é o $2006.
	void writeToPPUADDR(Byte value);

	// Interface com a CPU
	void cpuWrite(uint16_t addr, Byte data);

	Byte cpuRead(uint16_t addr);

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

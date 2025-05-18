#include "PPU.h"
#include "Tipos.h"

void PPUCTRL::write(Byte value) {
	control = value;
}

bool PPUCTRL::isNMIEnabled() const {
	return (control & 0x80) != 0;
}

bool PPUCTRL::isMasterSlave() const {
	return (control & 0x40) != 0;
}

Byte PPUCTRL::getNameTableAddr() const {
	return control & 0x03;
}

Byte PPUSTATUS::read() {
	Byte result = status;
	status &= ~0x80;
	return result;
}

void PPUSTATUS::setVBlank(bool value) {
	if (value) {
		status |= 0x80;
	}
	else {
		status &= ~0x80;
	}
}

Byte PPU::read(uint16_t address) {
	address &= 0x3FFF;

	if (address >= 0x2000 && address <= 0x3EFF) {
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

void PPU::write(uint16_t address, Byte value) {
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

uint16_t PPU::mirrorAddress(uint16_t address) {
	address = (address - 0x2000) % 0x1000;
	return address % 0x800;
}

void PPU::step() {
	dot++;

	if (dot > 340) {
		dot = 0;
		scanline++;

		if (scanline > 261) {
			scanline = 0;
		}
	}

	if (scanline == 241 && dot == 1) {
		status.setVBlank(true);
		if (ctrl.isNMIEnabled()) {
			// NMI
		}
	}

	if (scanline == 261 && dot == 1) {
		status.setVBlank(false);
	}

	if (scanline < 240) {
		renderScanline(scanline);
	}
}

void PPU::renderScanline(int scanline) {
	renderSprites(scanline);
}

void PPU::drawSpriteTile(Byte tileIndex, Byte x, Byte y, Byte attributes, int scanline) {
	// Placeholder
}

void PPU::renderSprites(int scanline) {
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

void PPU::doOAMDMA(const Byte* cpuMemoryPage) {
	for (int i = 0; i < 256; ++i) {
		OAM[i] = cpuMemoryPage[i];
	}
}

void PPU::writeToPPUADDR(Byte value) {
	if (!addressLatch) {
		ppuAddress = (value << 8) | (ppuAddress & 0x00FF);
		addressLatch = true;
	}
	else {
		ppuAddress = (ppuAddress & 0xFF00) | value;
		addressLatch = false;
	}
}

void PPU::cpuWrite(uint16_t addr, Byte data) {
	switch (addr & 0x0007) {
	case 0x0:
		ctrl.write(data);
		break;
	case 0x3:
		oamAddress = data;
		break;
	case 0x4:
		OAM[oamAddress] = data;
		oamAddress++;
		break;
	case 0x6:
		writeToPPUADDR(data);
		break;
	default:
		break;
	}
}

Byte PPU::cpuRead(uint16_t addr) {
	Byte data = 0x00;
	switch (addr & 0x0007) {
	case 0x2:
		data = status.read();
		addressLatch = false;
		break;
	case 0x4:
		data = OAM[oamAddress];
		break;
	default:
		break;
	}
	return data;
}

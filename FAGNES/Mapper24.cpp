#include "Mapper24.h"
#include <cstring>
#include "Tipos.h"
#include <vector>
#include <iostream>

Mapper24::Mapper24(int prgBanks, int chrBanks, const std::vector<Byte>& prgROM, const std::vector<Byte>& chrROM)
    : prgROM(prgROM), chrROM(chrROM), prgBankCount(prgBanks), chrBankCount(chrBanks) {

    chrIsRAM = (chrBanks == 0);
    if (chrIsRAM) {
        chrRAM.resize(0x2000);  // Aloca 8KB de CHR-RAM
    }

    std::memset(prgBankSelect, 0, sizeof(prgBankSelect));
    std::memset(chrBankSelect, 0, sizeof(chrBankSelect));

    prgFixedBankOffset = (prgROM.size() - 0x2000);  // Último banco fixo de 8KB

    syncPRGBanks();
    syncCHRBanks();
}

void Mapper24::syncPRGBanks() {
    prgBankOffsets[0] = (prgBankSelect[0] % (prgBankCount * 2)) * 0x2000;
    prgBankOffsets[1] = (prgBankSelect[1] % (prgBankCount * 2)) * 0x2000;
}

void Mapper24::syncCHRBanks() {
    for (int i = 0; i < 8; ++i) {
        chrBankOffsets[i] = chrBankSelect[i] * 0x0400;
    }
}

Byte Mapper24::cpuRead(DWord addr) {
    if (addr >= 0x8000 && addr <= 0x9FFF) {
        return prgROM[prgBankOffsets[0] + (addr - 0x8000)];
    }
    else if (addr >= 0xA000 && addr <= 0xBFFF) {
        return prgROM[prgBankOffsets[1] + (addr - 0xA000)];
    }
    else if (addr >= 0xC000 && addr <= 0xFFFF) {
        return prgROM[prgFixedBankOffset + (addr - 0xC000)];
    }
    return 0xFF;
}

void Mapper24::cpuWrite(DWord addr, Byte data) {
    if (addr >= 0x8000 && addr <= 0x9FFF) {
        prgBankSelect[0] = data & 0x1F;  // 5 bits
        syncPRGBanks();
    }
    else if (addr >= 0xA000 && addr <= 0xBFFF) {
        prgBankSelect[1] = data & 0x1F;
        syncPRGBanks();
    }
    else if (addr >= 0xB000 && addr <= 0xE00C) {
        uint8_t bank = ((addr - 0xB000) >> 1) & 0x07;  // De B000 até E00C
        chrBankSelect[bank] = data;
        syncCHRBanks();
    }
}

Byte Mapper24::ppuRead(DWord addr) {
    if (addr < 0x2000) {
        uint8_t bank = addr / 0x0400;
        uint16_t offset = addr % 0x0400;
        if (chrIsRAM) {
            return chrRAM[chrBankOffsets[bank] + offset];
        }
        else {
            return chrROM[chrBankOffsets[bank] + offset];
        }
    }
    return 0xFF;
}

void Mapper24::ppuWrite(DWord addr, Byte data) {
    if (addr < 0x2000 && chrIsRAM) {
        uint8_t bank = addr / 0x0400;
        uint16_t offset = addr % 0x0400;
        chrRAM[chrBankOffsets[bank] + offset] = data;
    }
}

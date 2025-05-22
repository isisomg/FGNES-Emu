#include "Mapper1.h"

Mapper1::Mapper1(Byte prg_Banks, Byte chr_Banks, const std::vector<Byte>& prg, const std::vector<Byte>& chr) {
    this->prgBanks = prg_Banks;
    this->chrBanks = chr_Banks;
    this->prgROM = prg;
    this->chrROM = chr;
}

Byte Mapper1::cpuRead(DWord addr) {
    if (addr >= 0x8000) {
        uint32_t mappedAddr = getPRGAddress(addr);
        return prgROM[mappedAddr % prgROM.size()];
    }
    return 0x00;
}

void Mapper1::cpuWrite(DWord addr, Byte data) {
    if (addr >= 0x8000) {
        writeRegister(addr, data);
    }
}

Byte Mapper1::ppuRead(DWord addr) {
    if (addr < 0x2000) {
        uint32_t mappedAddr = getCHRAddress(addr);
        return chrROM[mappedAddr % chrROM.size()];
    }
    return 0x00;
}

void Mapper1::ppuWrite(DWord addr, Byte data) {
    if (addr < 0x2000) {
        if (chrROM.size() == 0) {
            // CHR RAM
            chrROM[addr] = data;
        }
        else {
            std::cerr << "CHR-ROM não é gravável.\n";
        }
    }
}

void Mapper1::writeRegister(DWord addr, Byte data) {
    if (data & 0x80) {
        // Reset do shift register
        shiftRegister = 0x10;
        control |= 0x0C; // Modo seguro: PRG switch para 32K
        return;
    }

    bool complete = (shiftRegister & 1);
    shiftRegister >>= 1;
    shiftRegister |= (data & 1) << 4;

    if (complete) {
        uint16_t reg = (addr >> 13) & 0x03;
        switch (reg) {
        case 0:
            control = shiftRegister & 0x1F;
            break;
        case 1:
            chrBank0 = shiftRegister & 0x1F;
            break;
        case 2:
            chrBank1 = shiftRegister & 0x1F;
            break;
        case 3:
            prgBank = shiftRegister & 0x0F;
            break;
        }
        shiftRegister = 0x10; // Reset após escrita
    }
}

uint32_t Mapper1::getPRGAddress(DWord addr) {
    uint32_t mode = (control >> 2) & 0x03;
    uint32_t bank = prgBank & (prgBanks * 2 - 1);

    if (mode == 0 || mode == 1) {
        // 32 KB mode
        return (bank & 0xFE) * 0x4000 + (addr & 0x7FFF);
    }
    else if (mode == 2) {
        // Fix lower 16KB to 0x8000
        if (addr < 0xC000) {
            return 0x0000 + (addr & 0x3FFF);
        }
        else {
            return bank * 0x4000 + (addr & 0x3FFF);
        }
    }
    else {
        // Fix upper 16KB to 0xC000
        if (addr < 0xC000) {
            return bank * 0x4000 + (addr & 0x3FFF);
        }
        else {
            return (prgBanks * 2 - 1) * 0x4000 + (addr & 0x3FFF);
        }
    }
}

uint32_t Mapper1::getCHRAddress(DWord addr) {
    if ((control & 0x10) == 0) {
        // 8KB mode
        return (chrBank0 & 0x1E) * 0x1000 + (addr & 0x1FFF);
    }
    else {
        // 4KB mode
        if (addr < 0x1000) {
            return chrBank0 * 0x1000 + (addr & 0x0FFF);
        }
        else {
            return chrBank1 * 0x1000 + (addr & 0x0FFF);
        }
    }
}

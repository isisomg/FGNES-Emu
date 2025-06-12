#pragma once
#include "Mapper.h"
#include "Tipos.h"
#include <vector>
#include <iostream>
#include "PPU.h"

class Mapper1 : public Mapper {
public:
    Mapper1(Byte prgBanks, Byte chrBanks, const std::vector<Byte>& prgROM, const std::vector<Byte>& chrROM, MirroringSelect mirroringselect, PPU* ppu)
        : prgROM(prgROM), chrROM(chrROM), numPRGBanks(prgBanks), numCHRBanks(chrBanks), mirroringselect(mirroringselect), ppu(ppu){
        shiftRegister = 0x10; // valor inicial com bit 4 setado
        control = 0x0C;       // PRG mode = fixar último banco
        chrBank0 = 0;
        chrBank1 = 0;
        prgBank = 0;
        ppu->setMirroring(mirroringselect);
    }

    void cpuWrite(DWord addr, Byte data) override {
        if (addr < 0x8000) return;

        // Se o bit 7 está setado, resetamos o shift register
        if (data & 0x80) {
            shiftRegister = 0x10;
            control |= 0x0C; // PRG ROM bank mode = fixar último banco
            return;
        }

        bool complete = shiftRegister & 1;
        shiftRegister >>= 1;
        shiftRegister |= (data & 1) << 4;

        if (complete) {
            applyRegister(addr, shiftRegister);
            shiftRegister = 0x10; // resetar após escrever
        }
    }

    Byte cpuRead(DWord addr) override {
        if (addr >= 0x8000 && addr <= 0xFFFF) {
            if ((control & 0x0C) == 0x0C) {
                // PRG mode 3: fixar último banco em 0xC000-0xFFFF
                if (addr < 0xC000) {
                    uint32_t offset = (prgBank * 0x4000) + (addr - 0x8000);
                    return prgROM[offset % prgROM.size()];
                }
                else {
                    uint32_t offset = ((numPRGBanks - 1) * 0x4000) + (addr - 0xC000);
                    return prgROM[offset % prgROM.size()];
                }
            }
            else if ((control & 0x0C) == 0x08) {
                // PRG mode 2: fixar 1º banco
                if (addr < 0xC000) {
                    uint32_t offset = 0 + (addr - 0x8000);
                    return prgROM[offset % prgROM.size()];
                }
                else {
                    uint32_t offset = (prgBank * 0x4000) + (addr - 0xC000);
                    return prgROM[offset % prgROM.size()];
                }
            }
            else {
                // PRG mode 0/1: 32KB banco em 0x8000
                uint32_t bank = (prgBank & 0x0E) >> 1;
                uint32_t offset = (bank * 0x8000) + (addr - 0x8000);
                return prgROM[offset % prgROM.size()];
            }
        }

        return 0xFF;
    }

    Byte ppuRead(DWord addr) override {
        if (addr < 0x2000) {
            if (control & 0x10) {
                // CHR mode 1: dois bancos de 4KB
                if (addr < 0x1000) {
                    uint32_t offset = (chrBank0 * 0x1000) + addr;
                    return chrROM[offset % chrROM.size()];
                }
                else {
                    uint32_t offset = (chrBank1 * 0x1000) + (addr - 0x1000);
                    return chrROM[offset % chrROM.size()];
                }
            }
            else {
                // CHR mode 0: um banco de 8KB
                uint32_t bank = (chrBank0 & 0x1E);
                uint32_t offset = (bank * 0x1000) + addr;
                return chrROM[offset % chrROM.size()];
            }
        }

        return 0xFF;
    }

    void ppuWrite(DWord addr, Byte data) override {
        // CHR-RAM pode escrever aqui — adicionar depois, se necessário
    }

private:
    PPU* ppu;
    MirroringSelect mirroringselect;
    void applyRegister(DWord addr, Byte value) {
        if (addr >= 0x8000 && addr <= 0x9FFF) {
            control = value;

            // Bits 0-1: controle de mirroring
            Byte mirror = control & 0b11;

            switch (mirror) {
            case 0:
                ppu->setMirroring(MirroringSelect::OneScreenLower);
                break;
            case 1:
                ppu->setMirroring(MirroringSelect::OneScreenUpper);
                break;
            case 2:
                ppu->setMirroring(MirroringSelect::Vertical);
                break;
            case 3:
                ppu->setMirroring(MirroringSelect::Horizontal);
                break;
            }
        }
        else if (addr >= 0xA000 && addr <= 0xBFFF) {
            chrBank0 = value;
        }
        else if (addr >= 0xC000 && addr <= 0xDFFF) {
            chrBank1 = value;
        }
        else if (addr >= 0xE000 && addr <= 0xFFFF) {
            prgBank = value & 0x0F;
        }
    }


    std::vector<Byte> prgROM;
    std::vector<Byte> chrROM;

    Byte control = 0x0C;
    Byte chrBank0 = 0;
    Byte chrBank1 = 0;
    Byte prgBank = 0;

    Byte shiftRegister = 0x10;

    Byte numPRGBanks = 0;
    Byte numCHRBanks = 0;
};

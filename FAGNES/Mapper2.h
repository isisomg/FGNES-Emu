#pragma once
#include "Mapper.h"
#include "Tipos.h"
#include <vector>
#include <iostream>
#include "PPU.h"

class Mapper2 : public Mapper {
public:
    Mapper2(Byte prgBanks, Byte chrBanks, const std::vector<Byte>& prgROM, const std::vector<Byte>& chrROM, MirroringSelect mirroringselect, PPU* ppu)
        : prgROM(prgROM), chrROM(chrROM), numPRGBanks(prgBanks), numCHRBanks(chrBanks), mirroringselect(mirroringselect), ppu(ppu)
    {
        if (chrROM.empty()) {
            // CHR-RAM: aloca 8 KB
            useChrRAM = true;
            chrRAM.resize(8192, 0);
        }
        else {
            this->chrROM = chrROM;
        }
        // Começa com banco 0 selecionado
        selectedBank = 0;

        ppu->setMirroring(mirroringselect);
    }

    Byte cpuRead(DWord addr) override {
        if (addr >= 0x8000 && addr <= 0xBFFF) {
            uint32_t offset = (static_cast<uint32_t>(selectedBank) * 0x4000) + (addr - 0x8000);
            return prgROM[offset % prgROM.size()];
        }
        else if (addr >= 0xC000 && addr <= 0xFFFF) {
            uint32_t bankIndex = static_cast<uint32_t>(numPRGBanks - 1);
            uint32_t offset = (bankIndex * 0x4000) + (addr - 0xC000);
            if (offset < prgROM.size()) {
                return prgROM[offset];
            }
            else {
                return 0xFF;
            }
        }
        return 0xFF;
    }




    void cpuWrite(DWord addr, Byte data) override {
        if (addr == 0xFFFC || addr == 0xFFFD) {
            uint32_t bankIndex = static_cast<uint32_t>(numPRGBanks - 1);
            uint32_t offset = (bankIndex * 0x4000) + (addr - 0xC000);
        }

        if (addr >= 0x8000 && addr <= 0xFFFF) {
            // Apenas os 3 bits inferiores são usados
            selectedBank = data & 0x07;

            // Proteção para não ultrapassar o número de bancos disponíveis
            if (selectedBank >= numPRGBanks - 1) {
                selectedBank = selectedBank % (numPRGBanks - 1);
            }
        }
    }


    Byte ppuRead(DWord addr) override {
        if (addr < 0x2000) {
            if (useChrRAM) {
                return chrRAM[addr];
            }
            else {
                return chrROM[addr % chrROM.size()];
            }
        }
        return 0xFF;
    }

    void ppuWrite(DWord addr, Byte data) override {
        // Só permite escrita se for CHR-RAM (tamanho == 0 ou não fornecido)
        if (addr < 0x2000 && chrROM.size() == 0) {
            chrRAM[addr] = data;
        }
    }
    Byte numPRGBanks = 1;

private:
    PPU* ppu;
    MirroringSelect mirroringselect;
    bool useChrRAM = true;
    std::vector<Byte> chrRAM;
    std::vector<Byte> prgROM;
    std::vector<Byte> chrROM;
    Byte selectedBank = 0;
    
    Byte numCHRBanks = 0;
};

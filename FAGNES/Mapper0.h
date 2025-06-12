#pragma once
#include "Mapper.h"
#include "Tipos.h"
#include <vector>
#include <iostream>
#include "PPU.h"

class Mapper0 : public Mapper {
private:
    Byte prgBanks;
    Byte chrBanks;
    std::vector<Byte> prgROM;
    std::vector<Byte> chrROM;
    MirroringSelect mirroringselect;
    PPU* ppu;
public:
    Mapper0(Byte prg_Banks, Byte chr_Banks, const std::vector<Byte>& prg, const std::vector<Byte>& chr, MirroringSelect mirroringselect, PPU* ppu);

    Byte cpuRead(DWord addr) override;
    void cpuWrite(DWord addr, Byte data) override;
    Byte ppuRead(DWord addr) override;
    void ppuWrite(DWord addr, Byte data) override;
};

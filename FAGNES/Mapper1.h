#pragma once
#include "Mapper.h"
#include <vector>
#include <cstdint>
#include <iostream>

class Mapper1 : public Mapper {
public:
    Mapper1(Byte prg_Banks, Byte chr_Banks, const std::vector<Byte>& prg, const std::vector<Byte>& chr);

    Byte cpuRead(DWord addr) override;
    void cpuWrite(DWord addr, Byte data) override;

    Byte ppuRead(DWord addr) override;
    void ppuWrite(DWord addr, Byte data) override;

private:
    Byte prgBanks;
    Byte chrBanks;
    std::vector<Byte> prgROM;
    std::vector<Byte> chrROM;

    // Registradores internos
    uint8_t shiftRegister = 0x10;
    uint8_t control = 0x0C;
    uint8_t chrBank0 = 0;
    uint8_t chrBank1 = 0;
    uint8_t prgBank = 0;

    // Estado de escrita serial
    void writeRegister(DWord addr, Byte data);

    // Helpers para calcular offsets de bancos
    uint32_t getPRGAddress(DWord addr);
    uint32_t getCHRAddress(DWord addr);
};

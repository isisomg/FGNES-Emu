#ifndef MAPPER24_H
#define MAPPER24_H

#include "Mapper.h"
#include <vector>
#include "Tipos.h"



class Mapper24 : public Mapper {
public:
    Mapper24(int prgBanks, int chrBanks, const std::vector<Byte>& prgROM, const std::vector<Byte>& chrROM);

    Byte cpuRead(DWord addr) override;
    void cpuWrite(DWord addr, Byte data) override;

    Byte ppuRead(DWord addr) override;
    void ppuWrite(DWord addr, Byte data) override;

private:
    const std::vector<Byte>& prgROM;
    const std::vector<Byte>& chrROM;

    std::vector<Byte> chrRAM;
    bool chrIsRAM;

    int prgBankCount;
    int chrBankCount;

    Byte prgBankSelect[2];    // 2 bancos PRG comutáveis de 8KB
    Byte chrBankSelect[8];    // 8 bancos CHR comutáveis de 1KB

    DWord prgBankOffsets[2];  // Endereços base dos bancos PRG
    DWord prgFixedBankOffset; // Endereço fixo para o último banco

    DWord chrBankOffsets[8];  // Endereços base dos bancos CHR

    void syncPRGBanks();
    void syncCHRBanks();
};

#endif

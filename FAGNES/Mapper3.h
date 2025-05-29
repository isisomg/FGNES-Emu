#pragma once

#include "Mapper.h"
#include "Tipos.h" // Supondo que Byte e DWord estão definidos aqui
#include <vector>
#include <iostream> // Para std::cerr em ppuWrite

class Mapper3 : public Mapper {
private:
    std::vector<Byte> prgROM_data;
    std::vector<Byte> chrROM_data;

    Byte num_prg_banks; // Número de bancos de PRG ROM de 16KB
    Byte num_chr_banks; // Número de bancos de CHR ROM de 8KB

    Byte chr_bank_select = 0; // Banco CHR atualmente selecionado, inicializado para o primeiro

    bool prg_16kb_mirrored = false; // Flag para otimizar leitura de PRG de 16KB

public:
    Mapper3(Byte prgBanks_count, Byte chrBanks_count,
        const std::vector<Byte>& prg_data,
        const std::vector<Byte>& chr_rom_data);

    Byte cpuRead(DWord addr) override;
    void cpuWrite(DWord addr, Byte data) override;

    Byte ppuRead(DWord addr) override;
    void ppuWrite(DWord addr, Byte data) override;

    // void reset() override; // Opcional
};

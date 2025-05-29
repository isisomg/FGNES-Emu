#include "Mapper3.h"

Mapper3::Mapper3(Byte prgBanks_count, Byte chrBanks_count,
    const std::vector<Byte>& prg_data,
    const std::vector<Byte>& chr_rom_data)
    : prgROM_data(prg_data),
    chrROM_data(chr_rom_data),
    num_prg_banks(prgBanks_count),
    num_chr_banks(chrBanks_count)
    // chr_bank_select já inicializado com 0 no .h
{
    if (num_prg_banks == 1) {
        prg_16kb_mirrored = true;
    }
    // Mapper 3 (CNROM) assume que chrBanks_count > 0 e chr_rom_data é válido.
    // Se chrBanks_count for 0, o comportamento de ppuRead será ler de um vetor vazio
    // ou fora dos limites se não houver verificações, o que é problemático.
    // Idealmente, o Cartucho não instanciaria Mapper3 se chrBanks_count fosse 0.
    // Para robustez, ppuRead deve verificar chrROM_data.empty().
}

// Opcional:
// void Mapper3::reset() {
//     chr_bank_select = 0; // Reseta para o primeiro banco CHR
// }

Byte Mapper3::cpuRead(DWord addr) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        DWord mapped_addr;
        if (prg_16kb_mirrored) {
            // Se 16KB PRG, espelha na janela de 32KB da CPU.
            // Ex: $8000-$BFFF e $C000-$FFFF ambos mapeiam para os mesmos 16KB da PRG ROM.
            // (addr - 0x8000) nos dá um offset de 0x0000 a 0x7FFF.
            // & 0x3FFF confina ao intervalo de 16KB (0x0000-0x3FFF).
            mapped_addr = (addr - 0x8000) & 0x3FFF;
        }
        else {
            // Se 32KB PRG (ou mais, embora incomum para CNROM), mapeamento direto.
            // (addr - 0x8000) mapeia $8000-$FFFF para $0000-$7FFF no vetor prgROM_data.
            mapped_addr = addr - 0x8000;
        }

        if (mapped_addr < prgROM_data.size()) {
            return prgROM_data[mapped_addr];
        }
    }
    return 0x00; // Endereço não mapeado pelo cartucho
}

void Mapper3::cpuWrite(DWord addr, Byte data) {
    // Escritas em qualquer lugar na faixa de PRG ROM ($8000-$FFFF)
    // controlam a seleção do banco CHR.
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        if (num_chr_banks > 0) {
            // O valor escrito 'data' seleciona o banco CHR.
            // A máscara efetiva depende do número de bancos CHR.
            // Por exemplo, se num_chr_banks for 4, (data & 0x03) selecionaria o banco.
            // Usar módulo é uma forma geral e segura.
            chr_bank_select = data % num_chr_banks;
        }
        // Se num_chr_banks for 0 (configuração inválida para CNROM), não faz nada.
    }
}

Byte Mapper3::ppuRead(DWord addr) {
    // PPU lê da CHR ROM na faixa $0000-$1FFF (8KB)
    if (addr <= 0x1FFF) {
        if (num_chr_banks == 0 || chrROM_data.empty()) {
            // Caso anômalo: CNROM sem bancos CHR ou dados CHR vazios.
            // Retorna um valor padrão ou simula comportamento específico.
            return 0x00;
        }
        // Calcula o offset no vetor chrROM_data baseado no banco selecionado.
        // Cada banco CHR tem 0x2000 bytes (8KB).
        DWord mapped_addr = (static_cast<DWord>(chr_bank_select) * 0x2000) + (addr & 0x1FFF);

        if (mapped_addr < chrROM_data.size()) {
            return chrROM_data[mapped_addr];
        }
        else {
            // Tentativa de ler além dos limites da CHR ROM (pode acontecer se
            // chr_bank_select for muito alto devido a um num_chr_banks incorreto
            // ou se a CHR ROM for menor que o esperado).
            // Retornar 0 é uma opção segura.
            return 0x00;
        }
    }
    return 0x00; // Endereço não mapeado na CHR
}

void Mapper3::ppuWrite(DWord addr, Byte data) {
    // Mapper 3 (CNROM) usa CHR ROM, que não é gravável pela PPU.
    if (addr <= 0x1FFF) {
        // Apenas loga a tentativa, não altera os dados.
        // std::cerr << "Mapper3: Tentativa de escrita na CHR ROM no endereço $" << std::hex << addr << std::endl;
    }
}
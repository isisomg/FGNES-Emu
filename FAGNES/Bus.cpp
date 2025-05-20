#include "Bus.h"

Byte Bus::read(DWord adr) {
	if (adr >= 0x0000 && adr <= 0x1FFF) { // Ta na memoria da CPU
		return Bus::memCPU[adr % 0x0800]; // Usa o modulo para nao precisar espelhar a memoria
	}
	else if (adr >= 0x2000 && adr <= 0x3FFF) {
		Bus::write(0x2002, 0xFF); // SIMULA VBLANK APENAS PARA TESTE ANTES DE FAZER A PPU
		return Bus::memPPU[(adr - 0x2000) % 8]; // Tira a necessidade de espelhar a cada 8 bytes.
	}
	else if (adr == 0x4016) {
		return controles.ler(); //leitura dos Controles
	}
	else if (adr >= 0x8000 && adr <= 0xFFFF) { // Ta na PGR ROM
		if (cartucho) // mapper0 read
			return cartucho->readPRG(adr);
	}
	return 0xFF; // Leitura fora do endereço. Retorna qualquer coisa 
}
void Bus::write(DWord adr, Byte dado) { // Usa o mesmo conceito de tirar o espelhamento
	if (adr >= 0x0000 && adr <= 0x1FFF) { 
		Bus::memCPU[adr % 0x0800] = dado; 
	}
	else if (adr >= 0x2000 && adr <= 0x3FFF) {
		Bus::memPPU[(adr - 0x2000) % 8] = dado; 
	}
	else if (adr == 0x4016) {
			controles.escreverStrobe(dado & 1); //escrita dos Controles
	}
	else if (adr >= 0x4000 && adr <= 0x4017) {
		if (apu) {
			apu->writeRegister(adr, dado); // você criará esse método na APU
		}
	}

}

int teste = 0;
bool Bus::checkNMI() { // IMPLEMENTAR CORRETAMENTE EM CONJUNTO COM PPU
	if (teste > 2000) {
		teste = 0;
		return true;
	}
	teste++;
	return false;
}

void Bus::setCartucho(Cartucho * cart) {
	cartucho = cart;
}

void Bus::setAPU(APU* novaAPU) {
	apu = novaAPU;
}
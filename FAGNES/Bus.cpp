#include "Bus.h"

void Bus::setControles(Controles* ctrl) {
	controles = ctrl;
}

Byte Bus::read(DWord adr) {
	if (adr >= 0x0000 && adr <= 0x1FFF) { // Ta na memoria da CPU
		return Bus::memCPU[adr % 0x0800]; // Usa o modulo para nao precisar espelhar a memoria
	}
	else if (adr >= 0x2000 && adr <= 0x3FFF) {
		if (ppu)
			return ppu->cpuRead((adr - 0x2000) % 8); // Acesso real � PPU
		else {										// FIZ ISSO PQ SE DER ERRADO VOLTA PRO VBLANK FAKE MAS PODE COMENTAR SE QUISER!
			Bus::write(0x2002, 0xFF); // VBLANK fake pro teste!!
			return Bus::memPPU[(adr - 0x2000) % 8];
		}
	}
	else if (adr == 0x4016) {
		return controles->ler(); //leitura dos Controles
	}
	else if (adr >= 0x8000 && adr <= 0xFFFF) { // Ta na PGR ROM
		if (cartucho) // mapper0 read
			return cartucho->readPRG(adr);
	}
	return 0xFF; // Leitura fora do endere�o. Retorna qualquer coisa 
}
void Bus::write(DWord adr, Byte dado) { // Usa o mesmo conceito de tirar o espelhamento
	if (adr >= 0x0000 && adr <= 0x1FFF) {
		Bus::memCPU[adr % 0x0800] = dado;
	}
	else if (adr >= 0x2000 && adr <= 0x3FFF) {
		if (ppu) {
			ppu->cpuWrite((adr - 0x2000) % 8, dado); // Escrita real na PPU
		}
		else {
			Bus::memPPU[(adr - 0x2000) % 8] = dado;		// MESMA COISA QUE NO READ!!!!!!!!!!
		}
	}
	else if (adr == 0x4014) { // DMA 
		if (ppu) {
			Byte page = dado;
			for (int i = 0; i < 256; i++) {
				Byte value = read(page * 0x100 + i);
				ppu->cpuWrite(0x04, value); // escreve em $2004
			}
		}
	}
	else if (adr == 0x4016) {
		controles->escreverStrobe(dado & 1); //escrita dos Controles
	}
	else if (adr >= 0x4000 && adr <= 0x4017) {
		//std::cout << "APU write $" << std::hex << adr << " = " << (int)dado << std::endl;
		if (apu)
			apu->writeRegister(adr, dado);
	}

}

bool Bus::checkNMI() {
	if (ppu){
		bool valor = ppu && ppu->isNMIRequested();
		return valor; // Vai verificar se a ppu ta requestando o NMI e tals
	}
	return false;
}

void Bus::setCartucho(Cartucho* cart) {
	cartucho = cart;
}

void Bus::setAPU(APU* novaAPU) {
	apu = novaAPU;
}

void Bus::setPPU(PPU* novaPPU) {
	ppu = novaPPU;
}
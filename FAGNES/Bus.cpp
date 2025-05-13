#include "Bus.h"

Byte Bus::read(DWord adr) {
	if (adr >= 0x0000 && adr <= 0x1FFF) { // Ta na memoria da CPU
		return Bus::memCPU[adr % 0x0800]; // Usa o modulo para nao precisar espelhar a memoria
	}
	else if (adr >= 0x2000 && adr <= 0x3FFF) {
		return Bus::memPPU[(adr - 0x2000) % 8]; // Tira a necessidade de espelhar a cada 8 bytes.
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
}

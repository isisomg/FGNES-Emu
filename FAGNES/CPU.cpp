#include "CPU.h"

void CPU::handleNMI() { // Implementar corretamente
	// 1. Empilha PC (em big endian)
	writeByte(0x0100 + SP, (PC >> 8) & 0xFF); // High byte
	SP--;
	writeByte(0x0100 + SP, PC & 0xFF);        // Low byte
	SP--;

	// 2. Empilha Status com B flag como 0 e bit 5 como 1
	Byte status = (N << 7) | (V << 6) | (1 << 5) | (0 << 4) |
		(D << 3) | (I << 2) | (Z << 1) | (C);
	writeByte(0x0100 + SP, status);
	SP--;

	// 3. Setar o flag I para inibir novas IRQs
	I = 1;

	// 4. Ler vetor NMI (em 0xFFFA/0xFFFB)
	Byte low = readByte(0xFFFA);
	Byte high = readByte(0xFFFB);
	PC = (high << 8) | low;
	std::cout << "NMI mudou o PC para " << std::hex << (int)PC << std::endl;
	//PC = 0x8220; // SMB. ARRUMAR
}


	void CPU::deveAtualizarGrafico(DWord adr) { // VERIFICAR POSICOES ORIA DE GRAFICO
		if (adr >= 0x0200 && adr <= 0x0600) {
			atualizarGrafico = true;
		}
	}

	// Reseta para os valores iniciais
	void CPU::inicializar(Bus* novoBus) {
		PC = 0x0600;
		SP = 0xFD;

		X = Y = A = 0x00;
		N = V = D = I = Z = C = 0;
		B = 1;

		// Para o nestest sla pq eh assim. Verificar
		B = 0; I = 1;

		bus = novoBus;
		iniciou = true;
	}

	Byte CPU::readByte(DWord adr) {
		return bus->read(adr);
	}
	void CPU::writeByte(DWord adr, Byte valor) {
		deveAtualizarGrafico(adr);
		bus->write(adr, valor);
	}

	Byte CPU::fetchByte() {
		if (PC >= 0xFFFF) {
			return 0xEA; // NOP
		}
		else {
			return readByte(PC++);
		}
	}
	void CPU::ajustaZ(Byte valor) {
		Z = (valor == 0);
	}
	void CPU::ajustaN(Byte valor) {
		N = (valor & 0x80) != 0;
	}

	// Instrucoes
	// Mais detalhes em https://www.nesdev.org/wiki/Instruction_reference

	// ACCESS 
	void CPU::LDA(DWord adr) {
		A = readByte(adr);
		ajustaZ(A);
		ajustaN(A);
	}
	void CPU::STA(DWord adr) {
		writeByte(adr, A);
	}
	void CPU::LDX(DWord adr) {
		X = readByte(adr);
		ajustaZ(X);
		ajustaN(X);
	}
	void CPU::STX(DWord adr) {
		writeByte(adr, X);
	}
	void CPU::LDY(DWord adr) {
		Y = readByte(adr);
		ajustaZ(Y);
		ajustaN(Y);
	}
	void CPU::STY(DWord adr) {
		writeByte(adr, Y);
	}

	// TRANSFER
	void CPU::TAX() {
		X = A;
		ajustaN(X);
		ajustaZ(X);
	}
	void CPU::TXA() {
		A = X;
		ajustaN(A);
		ajustaZ(A);
	}
	void CPU::TAY() {
		Y = A;
		ajustaN(Y);
		ajustaZ(Y);
	}
	void CPU::TYA() {
		A = Y;
		ajustaN(A);
		ajustaZ(A);
	}

	// ARITHMETIC
	void CPU::ADC(Byte aux) {
		DWord soma = A + aux + C;

		C = soma > 0xFF;
		V = (((~(A ^ aux)) & (A ^ soma)) & 0x80) != 0;
		ajustaZ(soma);
		ajustaN(soma);

		A = soma & 0xFF;
	}
	void CPU::SBC(DWord adr) {
		Byte valor = readByte(adr);
		Byte carryIn = C ? 0 : 1; // Inversor

		DWord resultado16 = A - valor - carryIn;
		Byte resultado8 = resultado16 & 0xFF;

		C = (A >= (valor + carryIn));

		ajustaZ(resultado8);
		ajustaN(resultado8);

		V = (((A ^ valor) & (A ^ resultado8)) & 0x80) != 0;

		A = resultado8;
	}
	void CPU::INC(DWord adr) {
		Byte valor = readByte(adr);
		valor++;
		ajustaN(valor);
		ajustaZ(valor);
		writeByte(adr, valor);
	}
	void CPU::DEC(DWord adr) {
		Byte valor = readByte(adr);
		valor--;
		ajustaN(valor);
		ajustaZ(valor);
		writeByte(adr, valor);
	}
	void CPU::INX() {
		X++;
		ajustaZ(X);
		ajustaN(X);
	}
	void CPU::DEX() {
		X--;
		ajustaZ(X);
		ajustaN(X);
	}
	void CPU::INY() {
		Y++;
		ajustaZ(Y);
		ajustaN(Y);
	}
	void CPU::DEY() {
		Y--;
		ajustaZ(Y);
		ajustaN(Y);
	}

	// SHIFT
	void CPU::ASL(DWord adr, bool acumulador) {
		Byte valor;
		if (acumulador) {
			valor = A;
		}
		else {
			valor = readByte(adr);
		}

		Byte novoValor = valor << 1;
		ajustaZ(novoValor);
		ajustaN(novoValor);
		C = (valor & 0x80) != 0;

		if (acumulador) {
			A = novoValor;
		}
		else {
			writeByte(adr, novoValor);
		}
	}
	void CPU::LSR(DWord adr, bool acumulador) {

		Byte valor;
		if (acumulador) {
			valor = A;
		}
		else {
			valor = readByte(adr);
		}

		C = (valor & 0x01);

		Byte novoValor = valor >> 1;
		ajustaZ(novoValor);
		N = 0;

		if (acumulador) {
			A = novoValor;
		}
		else {
			writeByte(adr, novoValor);
		}
	}
	void CPU::ROL(DWord adr, bool acumulador) {
		Byte valor;
		if (acumulador) {
			valor = A;
		}
		else {
			valor = readByte(adr);
		}

		bool novoC = (valor & 0x80) != 0;
		valor = valor << 1;

		if (C) {
			valor = valor | 0x01;
		}

		C = novoC;
		ajustaZ(valor);
		ajustaN(valor);

		if (acumulador) {
			A = valor;
		}
		else {
			writeByte(adr, valor);
		}
	}
	void CPU::ROR(DWord adr, bool acumulador) {
		Byte valor;
		if (acumulador) {
			valor = A;
		}
		else {
			valor = readByte(adr);
		}

		bool novoC = (valor & 0x01) != 0;
		valor = valor >> 1;

		if (C) {
			valor = valor | 0x80;
		}

		C = novoC;
		ajustaZ(valor);
		ajustaN(valor);

		if (acumulador) {
			A = valor;
		}
		else {
			writeByte(adr, valor);
		}
	}

	// BITWISE
	void CPU::AND(DWord adr) {
		Byte valor = readByte(adr);
		A = A & valor;
		ajustaN(A);
		ajustaZ(A);
	}
	void CPU::ORA(DWord adr) {
		Byte valor = readByte(adr);
		A = A | valor;
		ajustaN(A);
		ajustaZ(A);
	}
	void CPU::EOR(DWord adr) {
		Byte valor = readByte(adr);
		A = A ^ valor;
		ajustaN(A);
		ajustaZ(A);
	}
	void CPU::BIT(DWord adr) { 
		Byte valor = readByte(adr);
		Byte resultado = A & valor;
		ajustaN(valor);
		ajustaZ(resultado);
		V = (valor & 0x40) != 0;
	}

	// COMPARE
	void CPU::CMP(DWord adr) {
		Byte valor = readByte(adr);
		Byte resultado = A - valor;
		C = (A >= valor);
		ajustaZ(resultado);
		ajustaN(resultado);

	}
	void CPU::CPX(DWord adr) {
		Byte valor = readByte(adr);
		Byte resultado = X - valor;
		C = (X >= valor);
		ajustaZ(resultado);
		ajustaN(resultado);
	}
	void CPU::CPY(DWord adr) {
		Byte valor = readByte(adr);
		Byte resultado = Y - valor;
		C = (Y >= valor);
		ajustaZ(resultado);
		ajustaN(resultado);
	}

	// BRANCH
	void CPU::BCC() {
		int8_t offset = static_cast<int8_t> (fetchByte()); // Faz cast por causa do sinal
		if (C == 0) {
			PC += offset;
		}
	}
	void CPU::BCS() {
		int8_t offset = static_cast<int8_t> (fetchByte()); // Faz cast por causa do sinal
		if (C == 1) {
			PC += offset;
		}
	}
	void CPU::BEQ() {
		int8_t offset = static_cast<int8_t> (fetchByte()); // Faz cast por causa do sinal
		if (Z == 1) {
			PC += offset;
		}
	}
	void CPU::BNE() {
		int8_t offset = static_cast<int8_t> (fetchByte()); // Faz cast por causa do sinal
		if (Z == 0) {
			PC += offset;
		}
	}
	void CPU::BPL() {
		int8_t offset = static_cast<int8_t> (fetchByte()); // Faz cast por causa do sinal
		if (N == 0) {
			PC += offset;
		}
	}
	void CPU::BMI() {
		int8_t offset = static_cast<int8_t> (fetchByte()); // Faz cast por causa do sinal
		if (N == 1) {
			PC += offset;
		}
	}
	void CPU::BVC() {
		int8_t offset = static_cast<int8_t> (fetchByte()); // Faz cast por causa do sinal
		if (V == 0) {
			PC += offset;
		}
	}
	void CPU::BVS() {
		int8_t offset = static_cast<int8_t> (fetchByte()); // Faz cast por causa do sinal
		if (V == 1) {
			PC += offset;
		}
	}

	// JUMP
	void CPU::JMP(DWord adr, bool absoluto) {
		if (absoluto) {
			PC = adr;
		}
		else {
			PC = indirect();
		}
	}
	void CPU::JSR(DWord adr) {
		// Decrementa SP e empilha o endere�o de retorno (PC - 1), high byte depois low byte
		DWord returnAdr = PC - 1;

		writeByte(0x0100 + SP--, (returnAdr >> 8) & 0xFF); // high byte
		writeByte(0x0100 + SP--, returnAdr & 0xFF);     // low byte

		PC = adr;
	}
	void CPU::RTS() {
		SP++;
		Byte low = readByte(0x0100 + SP);
		SP++;
		Byte high = readByte(0x0100 + SP);

		DWord returnAddress = (high << 8) | low;

		PC = returnAddress + 1;
	}
	void CPU::BRK() { // AJUSTAR

		//PC = 0xFFFF; // TESTE
		//return; // TESTE

		// Empilha o endere�o de retorno (PC + 1) na pilha
		DWord returnAddress = PC + 1;

		// Empilha o byte de retorno
		writeByte(0x0100 + SP--, (returnAddress >> 8) & 0xFF);  // Empilha o byte alto
		writeByte(0x0100 + SP--, returnAddress & 0xFF);         // Empilha o byte baixo

		// Empilha o status de flags
		Byte status = (N << 7) | (V << 6) | (B << 4) | (D << 3) | (I << 2) | (Z << 1) | C;
		writeByte(0x0100 + SP--, status);

		// Desativa interrup��es (I = 1) e seta B flag (B = 1)
		I = 1;  // Desabilita interrup��es
		B = 1;  // Setar a flag de Break

		// O endere�o de interrup��o de BRK � 0xFFFE e 0xFFFF
		PC = (readByte(0xFFFE) | (readByte(0xFFFF) << 8));

	}
	void CPU::RTI() {
		// Desempilha o byte de status (flags) primeiro
		Byte status = readByte(0x0100 + ++SP);

		// Restaura as flags do processador
		C = status & 0x01;
		Z = (status >> 1) & 0x01;
		I = (status >> 2) & 0x01;
		D = (status >> 3) & 0x01;
		B = (status >> 4) & 0x01;
		V = (status >> 6) & 0x01;
		N = (status >> 7) & 0x01;

		// Desempilha o endere�o de retorno (low byte primeiro, depois high byte)
		Byte low = readByte(0x0100 + ++SP);
		Byte high = readByte(0x0100 + ++SP);

		// Restaura o PC
		PC = (high << 8) | low;
	}

	// STACK
	void CPU::PHA() {
		writeByte(0x0100 + SP, A);
		SP--;
	}
	void CPU::PLA() {
		SP++;
		A = readByte(0x0100 + SP);
		ajustaZ(A);
		ajustaN(A);
	}

	void CPU::PHP() {
		Byte status = 0;
		status |= (N << 7);
		status |= (V << 6);
		status |= (1 << 5); // Bit 5 sempre eh 1 quando empilhado
		status |= (1 << 4); // sempre 1 no PHP
		status |= (D << 3);
		status |= (I << 2);
		status |= (Z << 1);
		status |= (C << 0);
		writeByte(0x0100 + SP, status);
		SP--;
	}
	void CPU::PLP() {
		SP++;
		Byte status = readByte(0x0100 + SP);
		C = status & 0x01;
		Z = (status >> 1) & 0x01;
		I = (status >> 2) & 0x01;
		D = (status >> 3) & 0x01;
		B = (status >> 4) & 0x01;
		// Bit 5 � geralmente ignorado (sempre 1), pode pular
		V = (status >> 6) & 0x01;
		N = (status >> 7) & 0x01;
	}


	void CPU::TXS() {
		SP = X;
	}
	void CPU::TSX() {
		X = SP;
		ajustaN(X);
		ajustaZ(X);
	}

	// FLAGS
	void CPU::CLC() {
		C = 0;
	}
	void CPU::SEC() {
		C = 1;
	}
	void CPU::CLI() {
		I = 0;
	}
	void CPU::SEI() {
		I = 1;
	}
	void CPU::CLD() {
		D = 0;
	}
	void CPU::SED() {
		D = 1;
	}
	void CPU::CLV() {
		V = 0;
	}

	// OTHER
	void CPU::NOP() {
		;
	}


	// Modos de endere�amento 
	// Immediate - Retorna o valor
	Byte CPU::immediate() {
		return fetchByte();
	}
	// ZeroPage - Retorna o address
	Byte CPU::zeropage() {
		return fetchByte();
	}
	// ZeroPage,X - Retorna o address
	Byte CPU::zeropageX() {
		Byte adrBase = fetchByte();
		Byte adr = (adrBase + X) & 0xFF;
		return adr;
	}
	// ZeroPage,Y - Retorna o address
	Byte CPU::zeropageY() {
		Byte adrBase = fetchByte();
		Byte adr = (adrBase + Y) & 0xFF;
		return adr;
	}
	// Absolute - Retorna o address
	DWord CPU::absolute() {
		Byte adrBase = fetchByte();
		DWord adr = (fetchByte() << 8) | adrBase;
		return adr;
	}
	// Absolute X - Retorna o address
	DWord CPU::absoluteX() {
		Byte adrBase = fetchByte();
		DWord adrCompleto = (fetchByte() << 8) | adrBase;
		DWord adr = adrCompleto + X;
		return adr;
	}
	// Absolute Y - Retorna o address
	DWord CPU::absoluteY() {
		Byte adrBase = fetchByte();
		DWord adrCompleto = (fetchByte() << 8) | adrBase;
		DWord adr = adrCompleto + Y;
		return adr;
	}
	// Indirect - Retorna o adr
	DWord CPU::indirect() {
		Byte low = fetchByte();
		Byte high = fetchByte();
		DWord addr = (high << 8) | low;

		// Emula o bug do 6502: se o addr cruza p�gina (como 0x02FF), n�o pega 0x0300, pega 0x0200 + wrap
		Byte lo = readByte(addr);
		Byte hi = readByte((addr & 0xFF00) | ((addr + 1) & 0x00FF));

		return (hi << 8) | lo;
	}

	// Indirect X - Retorna o adr
	DWord CPU::indirectX() {
		Byte base = fetchByte();
		Byte ptr = (base + X) & 0xFF;

		Byte lo = readByte(ptr);
		Byte hi = readByte((ptr + 1) & 0xFF);

		return (hi << 8) | lo;
	}

	// Indirect Y - Retorna o adr
	DWord CPU::indirectY() {
		Byte base = fetchByte();
		Byte lo = readByte(base);
		Byte hi = readByte((base + 1) & 0xFF); 

		DWord addr = (hi << 8) | lo;
		addr = (addr + Y) & 0xFFFF;

		return addr;
	}


	// fetch - decode - execute
	void CPU::executar() {
		if (bus->checkNMI()) {
			handleNMI();
		}
		Byte op = fetchByte();
		switch (op)
		{

			// CASOS ADC ********************************************
		case 0x69: // ADC Immediate
		{
			ADC(immediate());
			break;
		}

		case 0x65: // ADC ZeroPage
		{
			Byte valorAdr = zeropage();
			ADC(readByte(valorAdr));
			break;
		}

		case 0x75: // ADC ZeroPage,X
		{
			Byte valorAdr = zeropageX();
			ADC(readByte(valorAdr));
			break;
		}

		case 0x6D: // ADC Absolute
		{
			DWord valorAdr = absolute();
			ADC(readByte(valorAdr));
			break;
		}

		case 0x7D: // ADC Absolute X
		{
			DWord valorAdr = absoluteX();
			ADC(readByte(valorAdr));
			break;
		}

		case 0x79: // ADC Absolute Y
		{
			DWord valorAdr = absoluteY();
			ADC(readByte(valorAdr));
			break;
		}

		case 0x61: // ADC Indirect X
		{
			DWord valorAdr = indirectX();
			ADC(readByte(valorAdr));
			break;
		}
		case 0x71: // ADC Indirect Y
		{
			DWord valorAdr = indirectY();
			ADC(readByte(valorAdr));
			break;
		}


		// CASOS LDA ********************************************
		case 0xA9:
		{
			LDA(PC++);
			break;
		}
		case 0xA5:
		{
			DWord adr = zeropage();
			LDA(adr);
			break;
		}
		case 0xB5:
		{
			DWord adr = zeropageX();
			LDA(adr);
			break;
		}
		case 0xAD:
		{
			DWord adr = absolute();
			LDA(adr);
			break;
		}
		case 0xBD:
		{
			DWord adr = absoluteX();
			LDA(adr);
			break;
		}
		case 0xB9:
		{
			DWord adr = absoluteY();
			LDA(adr);
			break;
		}
		case 0xA1:
		{
			DWord adr = indirectX();
			LDA(adr);
			break;
		}
		case 0xB1:
		{
			DWord adr = indirectY();
			LDA(adr);
			break;
		}

		// CASOS LDX ********************************************
		case 0xA2:
		{
			LDX(PC++);
			break;
		}
		case 0xA6:
		{
			DWord adr = zeropage();
			LDX(adr);
			break;
		}
		case 0xB6:
		{
			DWord adr = zeropageY();
			LDX(adr);
			break;
		}
		case 0xAE:
		{
			DWord adr = absolute();
			LDX(adr);
			break;
		}
		case 0xBE:
		{
			DWord adr = absoluteY();
			LDX(adr);
			break;
		}

		// CASOS LDY ********************************************
		case 0xA0:
		{
			LDY(PC++);
			break;
		}
		case 0xA4:
		{
			DWord adr = zeropage();
			LDY(adr);
			break;
		}
		case 0xB4:
		{
			DWord adr = zeropageX();
			LDY(adr);
			break;
		}
		case 0xAC:
		{
			DWord adr = absolute();
			LDY(adr);
			break;
		}
		case 0xBC:
		{
			DWord adr = absoluteX();
			LDY(adr);
			break;
		}

		// CASO SEC
		case 0x38:
		{
			SEC();
			break;
		}
		// CASO CLC
		case 0x18:
		{
			CLC();
			break;
		}
		// CASO CLD
		case 0xD8:
		{
			CLD();
			break;
		}
		// CASO CLI
		case 0x58:
		{
			CLI();
			break;
		}
		// CASO CLV
		case 0xB8:
		{
			CLV();
			break;
		}

		// CASOS BRK
		case 0x00:
		{
			BRK();
			break;
		}
		// CASO DEX
		case 0xCA:
		{
			DEX();
			break;
		}
		// CASO DEY
		case 0x88:
		{
			DEY();
			break;
		}

		// CASOS STA ********************************************
		case 0x85:
		{
			DWord adr = zeropage();
			STA(adr);
			break;
		}
		case 0x95:
		{
			DWord adr = zeropageX();
			STA(adr);
			break;
		}
		case 0x8D:
		{
			DWord adr = absolute();
			STA(adr);
			break;
		}
		case 0x9D:
		{
			DWord adr = absoluteX();
			STA(adr);
			break;
		}
		case 0x99:
		{
			DWord adr = absoluteY();
			STA(adr);
			break;
		}
		case 0x81:
		{
			DWord adr = indirectX();
			STA(adr);
			break;
		}
		case 0x91:
		{
			DWord adr = indirectY();
			STA(adr);
			break;
		}
		// CASOS STX ********************************************
		case 0x86:
		{
			STX(zeropage());
			break;
		}
		case 0x96:
		{
			STX(zeropageY());
			break;
		}
		case 0x8E:
		{
			STX(absolute());
			break;
		}
		// CASOS STY ********************************************
		case 0x84:
		{
			STY(zeropage());
			break;
		}
		case 0x94:
		{
			STY(zeropageX());
			break;
		}
		case 0x8C:
		{
			STY(absolute());
			break;
		}

		// CASOS AND ********************************************
		case 0x29:
		{
			AND(PC++);
			break;
		}
		case 0x25:
		{
			AND(zeropage());
			break;
		}
		case 0x35:
		{
			AND(zeropageX());
			break;
		}
		case 0x2D:
		{
			AND(absolute());
			break;
		}
		case 0x3D:
		{
			AND(absoluteX());
			break;
		}
		case 0x39:
		{
			AND(absoluteY());
			break;
		}
		case 0x21:
		{
			AND(indirectX());
			break;
		}
		case 0x31:
		{
			AND(indirectY());
			break;
		}

		// CASOS EOR ********************************************
		case 0x49:
		{
			EOR(PC++);
			break;
		}
		case 0x45:
		{
			EOR(zeropage());
			break;
		}
		case 0x55:
		{
			EOR(zeropageX());
			break;
		}
		case 0x4D:
		{
			EOR(absolute());
			break;
		}
		case 0x5D:
		{
			EOR(absoluteX());
			break;
		}
		case 0x59:
		{
			EOR(absoluteY());
			break;
		}
		case 0x41:
		{
			EOR(indirectX());
			break;
		}
		case 0x51:
		{
			EOR(indirectY());
			break;
		}

		// CASO INX
		case 0xE8:
		{
			INX();
			break;
		}
		// CASO INY
		case 0xC8:
		{
			INY();
			break;
		}

		// CASOS JMP ********************************************
		case 0x4C:
		{
			JMP(absolute(), true);
			break;
		}
		case 0x6C:
		{
			JMP(0x00, false);
			break;
		}

		// CASO NOP
		case 0xEA:
		{
			NOP();
			break;
		}

		// CASOS ORA ********************************************
		case 0x09:
		{
			ORA(PC++);
			break;
		}
		case 0x05:
		{
			ORA(zeropage());
			break;
		}
		case 0x15:
		{
			ORA(zeropageX());
			break;
		}
		case 0x0D:
		{
			ORA(absolute());
			break;
		}
		case 0x1D:
		{
			ORA(absoluteX());
			break;
		}
		case 0x019:
		{
			ORA(absoluteY());
			break;
		}
		case 0x01:
		{
			ORA(indirectX());
			break;
		}
		case 0x11:
		{
			ORA(indirectY());
			break;
		}

		// CASO SED
		case 0xF8:
		{
			SED();
			break;
		}

		// CASO SEI
		case 0x78:
		{
			SEI();
			break;
		}

		// CASO TAX
		case 0xAA:
		{
			TAX();
			break;
		}
		// CASO TAY
		case 0xA8:
		{
			TAY();
			break;
		}
		// CASO TSX
		case 0xBA:
		{
			TSX();
			break;
		}
		// CASO TXA
		case 0x8A:
		{
			TXA();
			break;
		}
		// CASO TXS
		case 0x9A:
		{
			TXS();
			break;
		}
		// CASO TYA
		case 0x98:
		{
			TYA();
			break;
		}

		// CASOS CMP ********************************************
		case 0xC9:
		{
			CMP(PC++);
			break;
		}
		case 0xC5:
		{
			CMP(zeropage());
			break;
		}
		case 0xD5:
		{
			CMP(zeropageX());
			break;
		}
		case 0xCD:
		{
			CMP(absolute());
			break;
		}
		case 0xDD:
		{
			CMP(absoluteX());
			break;
		}
		case 0xD9:
		{
			CMP(absoluteY());
			break;
		}
		case 0xC1:
		{
			CMP(indirectX());
			break;
		}
		case 0xD1:
		{
			CMP(indirectY());
			break;
		}

		// CASOS CPX ********************************************
		case 0xE0:
		{
			CPX(PC++);
			break;
		}
		case 0xE4:
		{
			CPX(zeropage());
			break;
		}
		case 0xEC:
		{
			CPX(absolute());
			break;
		}

		// CASOS CPY ********************************************
		case 0xC0:
		{
			CPY(PC++);
			break;
		}
		case 0xC4:
		{
			CPY(zeropage());
			break;
		}
		case 0xCC:
		{
			CPY(absolute());
			break;
		}

		// CASOS INC ********************************************
		case 0xE6:
		{
			INC(zeropage());
			break;
		}
		case 0xF6:
		{
			INC(zeropageX());
			break;
		}
		case 0xEE:
		{
			INC(absolute());
			break;
		}
		case 0xFE:
		{
			INC(absoluteX());
			break;
		}

		// CASOS DEC ********************************************
		case 0xC6:
		{
			DEC(zeropage());
			break;
		}
		case 0xD6:
		{
			DEC(zeropageX());
			break;
		}
		case 0xCE:
		{
			DEC(absolute());
			break;
		}
		case 0xDE:
		{
			DEC(absoluteX());
			break;
		}

		// CASOS BIT ********************************************
		case 0x24:
		{
			BIT(zeropage());
			break;
		}
		case 0x2C:
		{
			BIT(absolute());
			break;
		}
		// CASOS ASL ********************************************
		case 0x0A:
		{
			ASL(PC, true);
			break;
		}
		case 0x06:
		{
			ASL(zeropage());
			break;
		}
		case 0x16:
		{
			ASL(zeropageX());
			break;
		}
		case 0x0E:
		{
			ASL(absolute());
			break;
		}
		case 0x1E:
		{
			ASL(absoluteX());
			break;
		}

		// CASOS LSR ********************************************
		case 0x4A:
		{
			LSR(PC, true);
			break;
		}
		case 0x46:
		{
			LSR(zeropage());
			break;
		}
		case 0x56:
		{
			LSR(zeropageX());
			break;
		}
		case 0x4E:
		{
			LSR(absolute());
			break;
		}
		case 0x5E:
		{
			LSR(absoluteX());
			break;
		}

		// CASOS ROL ********************************************
		case 0x2A:
		{
			ROL(PC, true);
			break;
		}
		case 0x26:
		{
			ROL(zeropage());
			break;
		}
		case 0x36:
		{
			ROL(zeropageX());
			break;
		}
		case 0x2E:
		{
			ROL(absolute());
			break;
		}
		case 0x3E:
		{
			ROL(absoluteX());
			break;
		}

		// CASOS ROR ********************************************
		case 0x6A:
		{
			ROR(PC, true);
			break;
		}
		case 0x66:
		{
			ROR(zeropage());
			break;
		}
		case 0x76:
		{
			ROR(zeropageX());
			break;
		}
		case 0x6E:
		{
			ROR(absolute());
			break;
		}
		case 0x7E:
		{
			ROR(absoluteX());
			break;
		}

		// CASOS SBC
		case 0xE9:
		{
			SBC(PC++);
			break;
		}
		case 0xE5:
		{
			SBC(zeropage());
			break;
		}
		case 0xF5:
		{
			SBC(zeropageX());
			break;
		}
		case 0xED:
		{
			SBC(absolute());
			break;
		}
		case 0xFD:
		{
			SBC(absoluteX());
			break;
		}
		case 0xF9:
		{
			SBC(absoluteY());
			break;
		}
		case 0xE1:
		{
			SBC(indirectX());
			break;
		}
		case 0xF1:
		{
			SBC(indirectY());
			break;
		}

		// Caso PLP
		case 0x28:
		{
			PLP();
			break;
		}
		// Caso PLA
		case 0x68:
		{
			PLA();
			break;
		}
		// Caso PHP
		case 0x08:
		{
			PHP();
			break;
		}
		// Caso PHA
		case 0x48:
		{
			PHA();
			break;
		}

		case 0x90:
		{
			BCC();
			break;
		}
		case 0xB0:
		{
			BCS();
			break;
		}
		case 0xF0:
		{
			BEQ();
			break;
		}
		case 0xD0:
		{
			BNE();
			break;
		}
		case 0x10:
		{
			BPL();
			break;
		}
		case 0x30:
		{
			BMI();
			break;
		}
		case 0x50:
		{
			BVC();
			break;
		}
		case 0x70:
		{
			BVS();
			break;
		}

		case 0x20:
		{
			JSR(absolute());
			break;
		}
		case 0x60:
		{
			RTS();
			break;
		}
		case 0x40:
		{
			RTI();
			break;
		}

		default:
			std::cout << "Instrucao nao encontrada " << std::hex << (int)op << std::endl;
			break;
		};
	}



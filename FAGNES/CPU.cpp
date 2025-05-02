#pragma once

#include <iostream>

#include "Memoria.cpp"

using Byte = unsigned char;
using DWord = unsigned short;

struct CPU
{

	DWord PC; // Program Counter
	DWord SP; // Stack Pointer

	// Registradores
	Byte A;
	Byte X;
	Byte Y;


	// Status Flag
	Byte N : 1; // Negativo
	Byte V : 1; // Overflow
	Byte B : 1; // Break
	Byte D : 1; // Decimal
	Byte I : 1; // Interrupt
	Byte Z : 1; // Zero
	Byte C : 1; // Carry

	bool atualizarGrafico = true;

	void deveAtualizarGrafico(DWord adr) { // VERIFICAR POSICOES MEMORIA DE GRAFICO
		if (adr >= 0x0200 && adr <= 0x0600) {
			atualizarGrafico = true;
		}
	}

	// Reseta para os valores iniciais
	void inicializar() {
		PC = 0x0600;
		SP = 0xFF;

		X = Y = A = 0x00;
		N = V = D = I = Z = C = 0;
		B = 1;
	}

	Byte readByte(Memoria& mem, DWord adr) {
		return mem[adr];
	}
	void writeByte(Memoria& mem, DWord adr, Byte valor) {
		deveAtualizarGrafico(adr);
		mem[adr] = valor;
	}

	Byte fetchByte(Memoria& mem) {
		if (PC >= 0xFFFF) {
			return 0xEA; // NOP
		}
		else {
			return mem[PC++];
		}
	}
	void ajustaZ(Byte valor) {
		Z = (valor == 0);
	}
	void ajustaN(Byte valor) {
		N = (valor & 0x80) != 0;
	}

	// Instru��es
	// Mais detalhes em https://www.nesdev.org/wiki/Instruction_reference

	// ACCESS 
	void LDA(Memoria& mem, DWord adr) {
		A = readByte(mem, adr);
		ajustaZ(A);
		ajustaN(A);
	}
	void STA(Memoria& mem, DWord adr) {
		writeByte(mem, adr, A);
	}
	void LDX(Memoria& mem, DWord adr) {
		X = readByte(mem, adr);
		ajustaZ(X);
		ajustaN(X);
	}
	void STX(Memoria& mem, DWord adr) {
		writeByte(mem, adr, X);
	}
	void LDY(Memoria& mem, DWord adr) {
		Y = readByte(mem, adr);
		ajustaZ(Y);
		ajustaN(Y);
	}
	void STY(Memoria& mem, DWord adr) {
		writeByte(mem, adr, Y);
	}

	// TRANSFER
	void TAX() {
		X = A;
		ajustaN(X);
		ajustaZ(X);
	}
	void TXA() {
		A = X;
		ajustaN(A);
		ajustaZ(A);
	}
	void TAY() {
		Y = A;
		ajustaN(Y);
		ajustaZ(Y);
	}
	void TYA() {
		A = Y;
		ajustaN(A);
		ajustaZ(A);
	}

	// ARITHMETIC
	void ADC(Byte aux) {
		DWord soma = A + aux + C;

		C = soma > 0xFF;
		V = (((~(A ^ aux)) & (A ^ soma)) & 0x80) != 0;
		ajustaZ(soma);
		ajustaN(soma);

		A = soma & 0xFF;
	}
	void SBC(Memoria& mem, DWord adr) {
		Byte valor = readByte(mem, adr);
		Byte carryIn = C ? 0 : 1; // Inversor

		DWord resultado16 = A - valor - carryIn;
		Byte resultado8 = resultado16 & 0xFF;

		C = (A >= (valor + carryIn));

		ajustaZ(resultado8);
		ajustaN(resultado8);

		V = (((A ^ valor) & (A ^ resultado8)) & 0x80) != 0;

		A = resultado8;
	}
	void INC(Memoria& mem, DWord adr) {
		Byte valor = readByte(mem, adr);
		valor++;
		ajustaN(valor);
		ajustaZ(valor);
		writeByte(mem, adr, valor);
	}
	void DEC(Memoria& mem, DWord adr) {
		Byte valor = readByte(mem, adr);
		valor--;
		ajustaN(valor);
		ajustaZ(valor);
		writeByte(mem, adr, valor);
	}
	void INX() {
		X++;
		ajustaZ(X);
		ajustaN(X);
	}
	void DEX() {
		X--;
		ajustaZ(X);
		ajustaN(X);
	}
	void INY() {
		Y++;
		ajustaZ(Y);
		ajustaN(Y);
	}
	void DEY() {
		Y--;
		ajustaZ(Y);
		ajustaN(Y);
	}

	// SHIFT
	void ASL(Memoria& mem, DWord adr, bool acumulador = false) {
		Byte valor;
		if (acumulador) {
			valor = A;
		}
		else {
			valor = readByte(mem, adr);
		}

		Byte novoValor = valor << 1;
		ajustaZ(novoValor);
		ajustaN(novoValor);
		C = (valor & 0x80) != 0;

		if (acumulador) {
			A = novoValor;
		}
		else {
			writeByte(mem, adr, novoValor);
		}
	}
	void LSR(Memoria& mem, DWord adr, bool acumulador = false) {

		Byte valor;
		if (acumulador) {
			valor = A;
		}
		else {
			valor = readByte(mem, adr);
		}

		C = (valor & 0x01);

		Byte novoValor = valor >> 1;
		ajustaZ(novoValor);
		N = 0;

		if (acumulador) {
			A = novoValor;
		}
		else {
			writeByte(mem, adr, novoValor);
		}
	}
	void ROL(Memoria& mem, DWord adr, bool acumulador = false) {
		Byte valor;
		if (acumulador) {
			valor = A;
		}
		else {
			valor = readByte(mem, adr);
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
			writeByte(mem, adr, valor);
		}
	}
	void ROR(Memoria& mem, DWord adr, bool acumulador = false) {
		Byte valor;
		if (acumulador) {
			valor = A;
		}
		else {
			valor = readByte(mem, adr);
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
			writeByte(mem, adr, valor);
		}
	}

	// BITWISE
	void AND(Memoria& mem, DWord adr) {
		Byte valor = readByte(mem, adr);
		A = A & valor;
		ajustaN(A);
		ajustaZ(A);
	}
	void ORA(Memoria& mem, DWord adr) {
		Byte valor = readByte(mem, adr);
		A = A | valor;
		ajustaN(A);
		ajustaZ(A);
	}
	void EOR(Memoria& mem, DWord adr) {
		Byte valor = readByte(mem, adr);
		A = A ^ valor;
		ajustaN(A);
		ajustaZ(A);
	}
	void BIT(Memoria& mem, DWord adr) { // REVISAR
		Byte valor = readByte(mem, adr);
		Byte resultado = A & valor;
		ajustaN(valor);
		ajustaZ(resultado);
		V = valor << 6;
	}

	// COMPARE
	void CMP(Memoria& mem, DWord adr) {
		Byte valor = readByte(mem, adr);
		C = (A >= valor);
		Z = (A == valor);
		ajustaN(A - valor);
	}
	void CPX(Memoria& mem, DWord adr) {
		Byte valor = readByte(mem, adr);
		C = (X >= valor);
		Z = (X == valor);
		ajustaN(X - valor);
	}
	void CPY(Memoria& mem, DWord adr) {
		Byte valor = readByte(mem, adr);
		C = (Y >= valor);
		Z = (Y == valor);
		ajustaN(Y - valor);
	}

	// BRANCH
	void BCC(Memoria& mem) {
		int8_t offset = static_cast<int8_t> (fetchByte(mem)); // Faz cast por causa do sinal
		if (C == 0) {
			PC += offset;
		}
	}
	void BCS(Memoria& mem) {
		int8_t offset = static_cast<int8_t> (fetchByte(mem)); // Faz cast por causa do sinal
		if (C == 1) {
			PC += offset;
		}
	}
	void BEQ(Memoria& mem) {
		int8_t offset = static_cast<int8_t> (fetchByte(mem)); // Faz cast por causa do sinal
		if (Z == 1) {
			PC += offset;
		}
	}
	void BNE(Memoria& mem) {
		int8_t offset = static_cast<int8_t> (fetchByte(mem)); // Faz cast por causa do sinal
		if (Z == 0) {
			PC += offset;
		}
	}
	void BPL(Memoria& mem) {
		int8_t offset = static_cast<int8_t> (fetchByte(mem)); // Faz cast por causa do sinal
		if (N == 0) {
			PC += offset;
		}
	}
	void BMI(Memoria& mem) {
		int8_t offset = static_cast<int8_t> (fetchByte(mem)); // Faz cast por causa do sinal
		if (N == 1) {
			PC += offset;
		}
	}
	void BVC(Memoria& mem) {
		int8_t offset = static_cast<int8_t> (fetchByte(mem)); // Faz cast por causa do sinal
		if (V == 0) {
			PC += offset;
		}
	}
	void BVS(Memoria& mem) {
		int8_t offset = static_cast<int8_t> (fetchByte(mem)); // Faz cast por causa do sinal
		if (V == 1) {
			PC += offset;
		}
	}

	// JUMP
	void JMP(Memoria& mem, DWord adr, bool absoluto = false) {
		if (absoluto) {
			PC = adr;
		}
		else {
			PC = readByte(mem, adr);
		}
	}
	void JSR(Memoria& mem, DWord adr) {
		// Decrementa SP e empilha o endere�o de retorno (PC - 1), high byte depois low byte
		DWord returnAdr = PC - 1;

		writeByte(mem, 0x0100 + SP--, (returnAdr >> 8) & 0xFF); // high byte
		writeByte(mem, 0x0100 + SP--, returnAdr & 0xFF);     // low byte

		PC = adr;
	}
	void RTS(Memoria& mem) {
		Byte low = readByte(mem, 0x0100 + ++SP);
		Byte high = readByte(mem, 0x0100 + ++SP);

		DWord returnAddress = (high << 8) | low;

		PC = returnAddress + 1;
	}
	void BRK(Memoria& mem) { // AJUSTAR

		PC = 0xFFFF; // TESTE
		return; // TESTE

		// Empilha o endere�o de retorno (PC + 1) na pilha
		DWord returnAddress = PC + 1;

		// Empilha o byte de retorno
		writeByte(mem, 0x0100 + SP--, (returnAddress >> 8) & 0xFF);  // Empilha o byte alto
		writeByte(mem, 0x0100 + SP--, returnAddress & 0xFF);         // Empilha o byte baixo

		// Empilha o status de flags
		Byte status = (N << 7) | (V << 6) | (B << 4) | (D << 3) | (I << 2) | (Z << 1) | C;
		writeByte(mem, 0x0100 + SP--, status);

		// Desativa interrup��es (I = 1) e seta B flag (B = 1)
		I = 1;  // Desabilita interrup��es
		B = 1;  // Setar a flag de Break

		// O endere�o de interrup��o de BRK � 0xFFFE e 0xFFFF
		PC = (readByte(mem, 0xFFFE) | (readByte(mem, 0xFFFF) << 8));

	}
	void RTI(Memoria& mem) {
		// Desempilha o byte de status (flags) primeiro
		Byte status = readByte(mem, 0x0100 + ++SP);

		// Restaura as flags do processador
		C = status & 0x01;
		Z = (status >> 1) & 0x01;
		I = (status >> 2) & 0x01;
		D = (status >> 3) & 0x01;
		B = (status >> 4) & 0x01;
		V = (status >> 6) & 0x01;
		N = (status >> 7) & 0x01;

		// Desempilha o endere�o de retorno (low byte primeiro, depois high byte)
		Byte low = readByte(mem, 0x0100 + ++SP);
		Byte high = readByte(mem, 0x0100 + ++SP);

		// Restaura o PC
		PC = (high << 8) | low;
	}

	// STACK
	void PHA(Memoria& mem) {
		writeByte(mem, 0x0100 + SP, A);
		SP--;
	}
	void PLA(Memoria& mem) {
		SP++;
		A = readByte(mem, 0x0100 + SP);
		ajustaN(A);
		ajustaZ(A);
	}
	void PHP(Memoria& mem) {
		Byte status = 0;
		status |= (N << 7);
		status |= (V << 6);
		status |= (1 << 5); // Bit 5 sempre � 1 quando empilhado
		status |= (B << 4);
		status |= (D << 3);
		status |= (I << 2);
		status |= (Z << 1);
		status |= (C << 0);
		writeByte(mem, 0x0100 + SP, status);
		SP--;
	}
	void PLP(Memoria& mem) {
		SP++;
		Byte status = readByte(mem, 0x0100 + SP);
		C = status & 0x01;
		Z = (status >> 1) & 0x01;
		I = (status >> 2) & 0x01;
		D = (status >> 3) & 0x01;
		B = (status >> 4) & 0x01;
		// Bit 5 � geralmente ignorado (sempre 1), pode pular
		V = (status >> 6) & 0x01;
		N = (status >> 7) & 0x01;
	}


	void TXS() {
		SP = X;
	}
	void TSX() {
		X = SP;
		ajustaN(X);
		ajustaZ(X);
	}

	// FLAGS
	void CLC() {
		C = 0;
	}
	void SEC() {
		C = 1;
	}
	void CLI() {
		I = 0;
	}
	void SEI() {
		I = 1;
	}
	void CLD() {
		D = 0;
	}
	void SED() {
		D = 1;
	}
	void CLV() {
		V = 0;
	}

	// OTHER
	void NOP() {
		;
	}


	// Modos de endere�amento 
	// Immediate - Retorna o valor
	Byte immediate(Memoria& mem) {
		return fetchByte(mem);
	}
	// ZeroPage - Retorna o address
	Byte zeropage(Memoria& mem) {
		return fetchByte(mem);
	}
	// ZeroPage,X - Retorna o address
	Byte zeropageX(Memoria& mem) {
		Byte adrBase = fetchByte(mem);
		Byte adr = (adrBase + X) & 0xFF;
		return adr;
	}
	// ZeroPage,Y - Retorna o address
	Byte zeropageY(Memoria& mem) {
		Byte adrBase = fetchByte(mem);
		Byte adr = (adrBase + Y) & 0xFF;
		return adr;
	}
	// Absolute - Retorna o address
	DWord absolute(Memoria& mem) {
		Byte adrBase = fetchByte(mem);
		DWord adr = (fetchByte(mem) << 8) | adrBase;
		return adr;
	}
	// Absolute X - Retorna o address
	DWord absoluteX(Memoria& mem) {
		Byte adrBase = fetchByte(mem);
		DWord adrCompleto = (fetchByte(mem) << 8) | adrBase;
		DWord adr = adrCompleto + X;
		return adr;
	}
	// Absolute Y - Retorna o address
	DWord absoluteY(Memoria& mem) {
		Byte adrBase = fetchByte(mem);
		DWord adrCompleto = (fetchByte(mem) << 8) | adrBase;
		DWord adr = adrCompleto + Y;
		return adr;
	}
	// Indirect - Retorna o adr
	DWord indirect(Memoria& mem) {
		Byte low = fetchByte(mem);
		Byte high = fetchByte(mem);
		DWord addr = (high << 8) | low;

		// Emula o bug do 6502: se o addr cruza p�gina (como 0x02FF), n�o pega 0x0300, pega 0x0200 + wrap
		Byte lo = readByte(mem, addr);
		Byte hi = readByte(mem, (addr & 0xFF00) | ((addr + 1) & 0x00FF));

		return (hi << 8) | lo;
	}

	// Indirect X - Retorna o adr
	DWord indirectX(Memoria& mem) {
		Byte base = fetchByte(mem);
		Byte ptr = (base + X) & 0xFF;

		Byte lo = readByte(mem, ptr);
		Byte hi = readByte(mem, (ptr + 1) & 0xFF);

		return (hi << 8) | lo;
	}

	// Indirect Y - Retorna o adr
	DWord indirectY(Memoria& mem) {
		Byte base = fetchByte(mem);
		Byte lo = readByte(mem, base);
		Byte hi = readByte(mem, (base + 1) & 0xFF); 

		DWord addr = (hi << 8) | lo;
		addr = (addr + Y) & 0xFFFF;

		return addr;
	}


	// fetch - decode - execute
	void executar(Memoria& mem) {
		Byte op = fetchByte(mem);
		switch (op)
		{

			// CASOS ADC ********************************************
		case 0x69: // ADC Immediate
		{
			ADC(immediate(mem));
			break;
		}

		case 0x65: // ADC ZeroPage
		{
			Byte valorAdr = zeropage(mem);
			ADC(readByte(mem, valorAdr));
			break;
		}

		case 0x75: // ADC ZeroPage,X
		{
			Byte valorAdr = zeropageX(mem);
			ADC(readByte(mem, valorAdr));
			break;
		}

		case 0x6D: // ADC Absolute
		{
			DWord valorAdr = absolute(mem);
			ADC(readByte(mem, valorAdr));
			break;
		}

		case 0x7D: // ADC Absolute X
		{
			DWord valorAdr = absoluteX(mem);
			ADC(readByte(mem, valorAdr));
			break;
		}

		case 0x79: // ADC Absolute Y
		{
			DWord valorAdr = absoluteY(mem);
			ADC(readByte(mem, valorAdr));
			break;
		}

		case 0x61: // ADC Indirect X
		{
			DWord valorAdr = indirectX(mem);
			ADC(readByte(mem, valorAdr));
			break;
		}
		case 0x71: // ADC Indirect Y
		{
			DWord valorAdr = indirectY(mem);
			ADC(readByte(mem, valorAdr));
			break;
		}


		// CASOS LDA ********************************************
		case 0xA9:
		{
			LDA(mem, PC++);
			break;
		}
		case 0xA5:
		{
			DWord adr = zeropage(mem);
			LDA(mem, adr);
			break;
		}
		case 0xB5:
		{
			DWord adr = zeropageX(mem);
			LDA(mem, adr);
			break;
		}
		case 0xAD:
		{
			DWord adr = absolute(mem);
			LDA(mem, adr);
			break;
		}
		case 0xBD:
		{
			DWord adr = absoluteX(mem);
			LDA(mem, adr);
			break;
		}
		case 0xB9:
		{
			DWord adr = absoluteY(mem);
			LDA(mem, adr);
			break;
		}
		case 0xA1:
		{
			DWord adr = indirectX(mem);
			LDA(mem, adr);
			break;
		}
		case 0xB1:
		{
			DWord adr = indirectY(mem);
			LDA(mem, adr);
			break;
		}

		// CASOS LDX ********************************************
		case 0xA2:
		{
			LDX(mem, PC++);
			break;
		}
		case 0xA6:
		{
			DWord adr = zeropage(mem);
			LDX(mem, adr);
			break;
		}
		case 0xB6:
		{
			DWord adr = zeropageY(mem);
			LDX(mem, adr);
			break;
		}
		case 0xAE:
		{
			DWord adr = absolute(mem);
			LDX(mem, adr);
			break;
		}
		case 0xBE:
		{
			DWord adr = absoluteY(mem);
			LDX(mem, adr);
			break;
		}

		// CASOS LDY ********************************************
		case 0xA0:
		{
			LDY(mem, PC++);
			break;
		}
		case 0xA4:
		{
			DWord adr = zeropage(mem);
			LDY(mem, adr);
			break;
		}
		case 0xB4:
		{
			DWord adr = zeropageX(mem);
			LDY(mem, adr);
			break;
		}
		case 0xAC:
		{
			DWord adr = absolute(mem);
			LDY(mem, adr);
			break;
		}
		case 0xBC:
		{
			DWord adr = absoluteX(mem);
			LDY(mem, adr);
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
			BRK(mem);
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
			DWord adr = zeropage(mem);
			STA(mem, adr);
			break;
		}
		case 0x95:
		{
			DWord adr = zeropageX(mem);
			STA(mem, adr);
			break;
		}
		case 0x8D:
		{
			DWord adr = absolute(mem);
			STA(mem, adr);
			break;
		}
		case 0x9D:
		{
			DWord adr = absoluteX(mem);
			STA(mem, adr);
			break;
		}
		case 0x99:
		{
			DWord adr = absoluteY(mem);
			STA(mem, adr);
			break;
		}
		case 0x81:
		{
			DWord adr = indirectX(mem);
			STA(mem, adr);
			break;
		}
		case 0x91:
		{
			DWord adr = indirectY(mem);
			STA(mem, adr);
			break;
		}
		// CASOS STX ********************************************
		case 0x86:
		{
			STX(mem, zeropage(mem));
			break;
		}
		case 0x96:
		{
			STX(mem, zeropageY(mem));
			break;
		}
		case 0x8E:
		{
			STX(mem, absolute(mem));
			break;
		}
		// CASOS STY ********************************************
		case 0x84:
		{
			STY(mem, zeropage(mem));
			break;
		}
		case 0x94:
		{
			STY(mem, zeropageX(mem));
			break;
		}
		case 0x8C:
		{
			STY(mem, absolute(mem));
			break;
		}

		// CASOS AND ********************************************
		case 0x29:
		{
			AND(mem, PC++);
			break;
		}
		case 0x25:
		{
			AND(mem, zeropage(mem));
			break;
		}
		case 0x35:
		{
			AND(mem, zeropageX(mem));
			break;
		}
		case 0x2D:
		{
			AND(mem, absolute(mem));
			break;
		}
		case 0x3D:
		{
			AND(mem, absoluteX(mem));
			break;
		}
		case 0x39:
		{
			AND(mem, absoluteY(mem));
			break;
		}
		case 0x21:
		{
			AND(mem, indirectX(mem));
			break;
		}
		case 0x31:
		{
			AND(mem, indirectY(mem));
			break;
		}

		// CASOS EOR ********************************************
		case 0x49:
		{
			EOR(mem, PC++);
			break;
		}
		case 0x45:
		{
			EOR(mem, zeropage(mem));
			break;
		}
		case 0x55:
		{
			EOR(mem, zeropageX(mem));
			break;
		}
		case 0x4D:
		{
			EOR(mem, absolute(mem));
			break;
		}
		case 0x5D:
		{
			EOR(mem, absoluteX(mem));
			break;
		}
		case 0x59:
		{
			EOR(mem, absoluteY(mem));
			break;
		}
		case 0x41:
		{
			EOR(mem, indirectX(mem));
			break;
		}
		case 0x51:
		{
			EOR(mem, indirectY(mem));
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
			JMP(mem, absolute(mem), true);
			break;
		}
		case 0x6C:
		{
			JMP(mem, indirect(mem));
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
			ORA(mem, PC++);
			break;
		}
		case 0x05:
		{
			ORA(mem, zeropage(mem));
			break;
		}
		case 0x15:
		{
			ORA(mem, zeropageX(mem));
			break;
		}
		case 0x0D:
		{
			ORA(mem, absolute(mem));
			break;
		}
		case 0x1D:
		{
			ORA(mem, absoluteX(mem));
			break;
		}
		case 0x019:
		{
			ORA(mem, absoluteY(mem));
			break;
		}
		case 0x01:
		{
			ORA(mem, indirectX(mem));
			break;
		}
		case 0x11:
		{
			ORA(mem, indirectY(mem));
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
			CMP(mem, PC++);
			break;
		}
		case 0xC5:
		{
			CMP(mem, zeropage(mem));
			break;
		}
		case 0xD5:
		{
			CMP(mem, zeropageX(mem));
			break;
		}
		case 0xCD:
		{
			CMP(mem, absolute(mem));
			break;
		}
		case 0xDD:
		{
			CMP(mem, absoluteX(mem));
			break;
		}
		case 0xD9:
		{
			CMP(mem, absoluteY(mem));
			break;
		}
		case 0xC1:
		{
			CMP(mem, indirectX(mem));
			break;
		}
		case 0xD1:
		{
			CMP(mem, indirectY(mem));
			break;
		}

		// CASOS CPX ********************************************
		case 0xE0:
		{
			CPX(mem, PC++);
			break;
		}
		case 0xE4:
		{
			CPX(mem, zeropage(mem));
			break;
		}
		case 0xEC:
		{
			CPX(mem, absolute(mem));
			break;
		}

		// CASOS CPY ********************************************
		case 0xC0:
		{
			CPY(mem, PC++);
			break;
		}
		case 0xC4:
		{
			CPY(mem, zeropage(mem));
			break;
		}
		case 0xCC:
		{
			CPY(mem, absolute(mem));
			break;
		}

		// CASOS INC ********************************************
		case 0xE6:
		{
			INC(mem, zeropage(mem));
			break;
		}
		case 0xF6:
		{
			INC(mem, zeropageX(mem));
			break;
		}
		case 0xEE:
		{
			INC(mem, absolute(mem));
			break;
		}
		case 0xFE:
		{
			INC(mem, absoluteX(mem));
			break;
		}

		// CASOS DEC ********************************************
		case 0xC6:
		{
			DEC(mem, zeropage(mem));
			break;
		}
		case 0xD6:
		{
			DEC(mem, zeropageX(mem));
			break;
		}
		case 0xCE:
		{
			DEC(mem, absolute(mem));
			break;
		}
		case 0xDE:
		{
			DEC(mem, absoluteX(mem));
			break;
		}

		// CASOS BIT ********************************************
		case 0x24:
		{
			BIT(mem, zeropage(mem));
			break;
		}
		case 0x2C:
		{
			BIT(mem, absolute(mem));
			break;
		}
		// CASOS ASL ********************************************
		case 0x0A:
		{
			ASL(mem, PC, true);
			break;
		}
		case 0x06:
		{
			ASL(mem, zeropage(mem));
			break;
		}
		case 0x16:
		{
			ASL(mem, zeropageX(mem));
			break;
		}
		case 0x0E:
		{
			ASL(mem, absolute(mem));
			break;
		}
		case 0x1E:
		{
			ASL(mem, absoluteX(mem));
			break;
		}

		// CASOS LSR ********************************************
		case 0x4A:
		{
			LSR(mem, PC, true);
			break;
		}
		case 0x46:
		{
			LSR(mem, zeropage(mem));
			break;
		}
		case 0x56:
		{
			LSR(mem, zeropageX(mem));
			break;
		}
		case 0x4E:
		{
			LSR(mem, absolute(mem));
			break;
		}
		case 0x5E:
		{
			LSR(mem, absoluteX(mem));
			break;
		}

		// CASOS ROL ********************************************
		case 0x2A:
		{
			ROL(mem, PC, true);
			break;
		}
		case 0x26:
		{
			ROL(mem, zeropage(mem));
			break;
		}
		case 0x36:
		{
			ROL(mem, zeropageX(mem));
			break;
		}
		case 0x2E:
		{
			ROL(mem, absolute(mem));
			break;
		}
		case 0x3E:
		{
			ROL(mem, absoluteX(mem));
			break;
		}

		// CASOS ROR ********************************************
		case 0x6A:
		{
			ROR(mem, PC, true);
			break;
		}
		case 0x66:
		{
			ROR(mem, zeropage(mem));
			break;
		}
		case 0x76:
		{
			ROR(mem, zeropageX(mem));
			break;
		}
		case 0x6E:
		{
			ROR(mem, absolute(mem));
			break;
		}
		case 0x7E:
		{
			ROR(mem, absoluteX(mem));
			break;
		}

		// CASOS SBC
		case 0xE9:
		{
			SBC(mem, PC++);
			break;
		}
		case 0xE5:
		{
			SBC(mem, zeropage(mem));
			break;
		}
		case 0xF5:
		{
			SBC(mem, zeropageX(mem));
			break;
		}
		case 0xED:
		{
			SBC(mem, absolute(mem));
			break;
		}
		case 0xFD:
		{
			SBC(mem, absoluteX(mem));
			break;
		}
		case 0xF9:
		{
			SBC(mem, absoluteY(mem));
			break;
		}
		case 0xE1:
		{
			SBC(mem, indirectX(mem));
			break;
		}
		case 0xF1:
		{
			SBC(mem, indirectY(mem));
			break;
		}

		// Caso PLP
		case 0x28:
		{
			PLP(mem);
			break;
		}
		// Caso PLA
		case 0x68:
		{
			PLA(mem);
			break;
		}
		// Caso PHP
		case 0x08:
		{
			PHP(mem);
			break;
		}
		// Caso PHA
		case 0x48:
		{
			PHA(mem);
			break;
		}

		case 0x90:
		{
			BCC(mem);
			break;
		}
		case 0xB0:
		{
			BCS(mem);
			break;
		}
		case 0xF0:
		{
			BEQ(mem);
			break;
		}
		case 0xD0:
		{
			BNE(mem);
			break;
		}
		case 0x10:
		{
			BPL(mem);
			break;
		}
		case 0x30:
		{
			BMI(mem);
			break;
		}
		case 0x50:
		{
			BVC(mem);
			break;
		}
		case 0x70:
		{
			BVS(mem);
			break;
		}

		case 0x20:
		{
			JSR(mem, absolute(mem));
			break;
		}
		case 0x60:
		{
			RTS(mem);
			break;
		}
		case 0x40:
		{
			RTI(mem);
			break;
		}

		default:
			std::cout << "Instrucao nao encontrada" << std::endl;
			break;
		};
	}

};


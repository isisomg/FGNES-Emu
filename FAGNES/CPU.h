#pragma once
#include "Memoria.h"
#include <iostream>

using Byte = unsigned char;
using DWord = unsigned short;

struct CPU {
	DWord PC; // Program Counter
	DWord SP; // Stack Pointer

	// Registradores
	Byte A;
	Byte X;
	Byte Y;

	// Status Flags
	Byte N : 1; // Negativo
	Byte V : 1; // Overflow
	Byte B : 1; // Break
	Byte D : 1; // Decimal
	Byte I : 1; // Interrupt
	Byte Z : 1; // Zero
	Byte C : 1; // Carry

	bool atualizarGrafico = true;

	void deveAtualizarGrafico(DWord adr);
	void inicializar();
	Byte readByte(Memoria& mem, DWord adr);
	void writeByte(Memoria& mem, DWord adr, Byte valor);
	Byte fetchByte(Memoria& mem);
	void ajustaZ(Byte valor);
	void ajustaN(Byte valor);

	void LDA(Memoria& mem, DWord adr);
	void STA(Memoria& mem, DWord adr);
	void LDX(Memoria& mem, DWord adr);
	void STX(Memoria& mem, DWord adr);
	void LDY(Memoria& mem, DWord adr);
	void STY(Memoria& mem, DWord adr);

	void TAX();
	void TXA();
	void TAY();
	void TYA();

	void ADC(Byte aux);
	void SBC(Memoria& mem, DWord adr);
	void INC(Memoria& mem, DWord adr);
	void DEC(Memoria& mem, DWord adr);
	void INX();
	void DEX();
	void INY();
	void DEY();

	void ASL(Memoria& mem, DWord adr, bool acumulador = false);
	void LSR(Memoria& mem, DWord adr, bool acumulador = false);
	void ROL(Memoria& mem, DWord adr, bool acumulador = false);
	void ROR(Memoria& mem, DWord adr, bool acumulador = false);
	
	void AND(Memoria& mem, DWord adr);
	void ORA(Memoria& mem, DWord adr);
	void EOR(Memoria& mem, DWord adr);
	void BIT(Memoria& mem, DWord adr);


	void CMP(Memoria& mem, DWord adr);
	void CPX(Memoria& mem, DWord adr);
	void CPY(Memoria& mem, DWord adr);

	void BCC(Memoria& mem);
	void BCS(Memoria& mem);
	void BEQ(Memoria& mem);
	void BNE(Memoria& mem);
	void BPL(Memoria& mem);
	void BMI(Memoria& mem);
	void BVC(Memoria& mem);
	void BVS(Memoria& mem);

	void JMP(Memoria& mem, DWord adr, bool absoluto = false);
	void JSR(Memoria& mem, DWord adr);
	void RTS(Memoria& mem);
	void BRK(Memoria& mem);
	void RTI(Memoria& mem);

	void PHA(Memoria& mem);
	void PLA(Memoria& mem);
	void PHP(Memoria& mem);
	void PLP(Memoria& mem);
	void TXS();
	void TSX();

	void CLC();
	void SEC();
	void CLI();
	void SEI();
	void CLD();
	void SED();
	void CLV();

	void NOP();

	Byte immediate(Memoria& mem);
	Byte zeropage(Memoria& mem);
	Byte zeropageX(Memoria& mem);
	Byte zeropageY(Memoria& mem);
	DWord absolute(Memoria& mem);
	DWord absoluteX(Memoria& mem);
	DWord absoluteY(Memoria& mem);
	DWord indirect(Memoria& mem);
	DWord indirectX(Memoria& mem);
	DWord indirectY(Memoria& mem);

	void executar(Memoria& mem);
};
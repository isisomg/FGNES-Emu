#pragma once
#include "Bus.h"
#include "Tipos.h"
#include <iostream>

class CPU {
public:
	DWord PC; // Program Counter
	DWord SP; // Stack Pointer

	Bus* bus;

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

	Byte getStatusRegister() {
		Byte p = 0x20; // Bit UNUSED � sempre 1 no NES

		p |= (C << 0);  // Carry
		p |= (Z << 1);  // Zero
		p |= (I << 2);  // Interrupt
		p |= (D << 3);  // Decimal
		p |= (B << 4);  // Break
		p |= (V << 6);  // Overflow
		p |= (N << 7);  // Negative

		return p;
	}
	
	bool iniciou = false;

	void handleNMI();
	void inicializar(Bus* novoBus);
	Byte readByte(DWord adr);
	void writeByte(DWord adr, Byte valor);
	Byte fetchByte();
	void ajustaZ(Byte valor);
	void ajustaN(Byte valor);

	void LDA(DWord adr);
	void STA(DWord adr);
	void LDX(DWord adr);
	void STX(DWord adr);
	void LDY(DWord adr);
	void STY(DWord adr);

	void TAX();
	void TXA();
	void TAY();
	void TYA();

	void ADC(Byte aux);
	void SBC(DWord adr);
	void INC(DWord adr);
	void DEC(DWord adr);
	void INX();
	void DEX();
	void INY();
	void DEY();

	void ASL(DWord adr, bool acumulador = false);
	void LSR(DWord adr, bool acumulador = false);
	void ROL(DWord adr, bool acumulador = false);
	void ROR(DWord adr, bool acumulador = false);
	
	void AND(DWord adr);
	void ORA(DWord adr);
	void EOR(DWord adr);
	void BIT(DWord adr);


	void CMP(DWord adr);
	void CPX(DWord adr);
	void CPY(DWord adr);

	int BCC();
	int BCS();
	int BEQ();
	int BNE();
	int BPL();
	int BMI();
	int BVC();
	int BVS();

	void JMP(DWord adr, bool absoluto = false);
	void JSR(DWord adr);
	void RTS();
	void BRK();
	void RTI();

	void PHA();
	void PLA();
	void PHP();
	void PLP();
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

	Byte immediate();
	Byte zeropage();
	Byte zeropageX();
	Byte zeropageY();
	DWord absolute();
	std::pair<DWord, bool> absoluteX();
	std::pair<DWord, bool> absoluteY();
	DWord indirect();
	DWord indirectX();
	std::pair<DWord, bool> indirectY();

	int executar(); // retorna o numero de ciclos utilizados
};
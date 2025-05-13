#include "Bus.h"

Byte Bus::read(DWord adr) {
	return Bus::mem[adr];
}
void Bus::write(DWord adr, Byte dado) {
	Bus::mem[adr] = dado;
}
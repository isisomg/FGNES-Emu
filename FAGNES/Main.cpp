//#include "CPU.cpp"
//#include "Memoria.cpp"
//
//#include <iostream>
//#include <fstream>
//
//int main() {
//
//	Memoria mem;
//	CPU cpu;
//	cpu.inicializar();
//
//	// Carrega os dados na memoria
//
//	std::string path = "snake.hex";
//	std::ifstream input(path, std::ios::in | std::ios::binary);
//	if (input.is_open() == false) {
//		return -1;
//	}
//	
//	input.seekg(0, input.end);
//	int tamanho = input.tellg();
//	input.seekg(0, input.beg);
//
//	for (int i = 0; i < tamanho; i++) {
//		input.read((char*)&mem[cpu.PC + i], sizeof(char));
//	}
//
//	for (int i = 0; i < tamanho; i++) {
//		printf("%x ", mem[cpu.PC + i]);
//	}
//	printf("\n");
//
//	// Dados carregados na memoria com sucesso
//
//	while (cpu.PC != 0xFFFF) {
//		cpu.executar(mem);
//		std::cout << cpu.PC << std::endl;
//	}
//
//	std::cout << "acabei" << std::endl;
//
//	return 0;
//}
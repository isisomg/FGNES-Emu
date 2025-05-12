#include "CPU.h"
#include "Memoria.h"
#include "SDL_Display.h"

#include <iostream>
#include <fstream>

void carregarROM(Memoria& mem, CPU& cpu) {
	std::string path = "snake.hex";
	std::ifstream input(path, std::ios::in | std::ios::binary);
	if (input.is_open() == false) {
		std::cout << "Erro ao carregar a ROM" << std::endl;
		exit(-1);
	}

	input.seekg(0, input.end);
	int tamanho = input.tellg();
	input.seekg(0, input.beg);

	for (int i = 0; i < tamanho; i++) {
		input.read((char*)&mem[cpu.PC + i], sizeof(char));
	}
}

int main(int argc, char* argv[]) {

	Memoria mem;
	CPU cpu;
	cpu.inicializar();

	SDL_Display display;
	display.init();

	carregarROM(mem, cpu);

	bool rodar = true;
	SDL_Event event;
	while (rodar) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) rodar = false;
			
			display.processarEntrada(event, mem);
		}


		if (cpu.PC == 0xFFFF) {
			break;
		}
		cpu.executar(mem);

		// mudar RNG
		mem[0x00FE] = rand() % 0xFF;

		if (cpu.atualizarGrafico == false) {
			continue;
		}
		cpu.atualizarGrafico = false;
		display.renderizar(mem);
		
	}

	display.destroy();
	return 0;
}
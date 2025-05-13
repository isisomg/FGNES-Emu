#include "CPU.h"
#include "Bus.h"
#include "SDL_Display.h"

#include <iostream>
#include <fstream>

void carregarROM(CPU& cpu) { // APENAS PARA TESTE
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
		Byte valor;
		input.read((char*) &valor, sizeof(char));
		cpu.bus->write(0x0600 + i, valor);
	}
}

int main(int argc, char* argv[]) {

	Bus* bus = new Bus();
	
	SDL_Display display;
	display.init(bus);
	
	CPU cpu;
	
	cpu.inicializar(bus);
	
	carregarROM(cpu);

	bool rodar = true;
	SDL_Event event;
	while (rodar) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) rodar = false;
			
			display.processarEntrada(event);
		}

		if (cpu.PC == 0xFFFF) {
			break;
		}
		cpu.executar();

		// mudar RNG
		cpu.writeByte(0x00FE, rand() % 0xFF);

		if (cpu.atualizarGrafico == false) {
			continue;
		}
		cpu.atualizarGrafico = false;
		display.renderizar();
		
	}

	display.destroy();
	delete bus;
	return 0;
}
#include "CPU.h"
#include "Bus.h"
#include "APU.h"
#include "SDL_Display.h"
#include "Cartucho.h"
#include <iostream>
#include <fstream>

const int ciclosPorFrame = 29781;

int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		std::cerr << "Erro ao inicializar SDL: " << SDL_GetError() << std::endl;
		return 1;
	}

	Bus* bus = new Bus();
	Cartucho cartucho;
	
	Controles controle;
	bus->setControles(&controle);
	
	CPU cpu;
	PPU ppu;
	APU apu;

	SDL_Display display;
	display.init(bus, &cartucho, &ppu);

	bus->setPPU(&ppu);
	// Ativa NMI
	ppu.cpuWrite(0x2000, 0x80);

	
	//cpu.inicializar(bus);

	//carregarROM(cpu); // remover quando for testar ROM. APENAS PARA TESTAR SNAKE
	//display.jogoRodando = true; //  APENAS PARA TESTAR SNAKE

	bool rodar = true;
	SDL_Event event;
	while (rodar) {
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT) rodar = false;

			display.processarEntrada(event);
		}

		if (display.jogoRodando) {
			if (cpu.iniciou == false) { // inicializacoes necessarias para o jogo
				cpu.inicializar(bus);
				cpu.PC = cartucho.adrPCinicial; 
				bus->setCartucho(&cartucho);
				ppu.carregarCHR(cartucho.chrROM);
			}

			if (cpu.PC == 0xFFFF) {
				break;
			}
			//guardarLinha(cpu.PC, cpu.A, cpu.X, cpu.Y, cpu.getStatusRegister(), cpu.SP); // PARA DEBUNG CPU
			int cpuCiclos = 0;
			while (cpuCiclos < ciclosPorFrame) { // RODA UM FRAME
				int ciclos = cpu.executar();
				cpuCiclos += ciclos;

				for (int c = 0; c < ciclos; ++c) {
					//apu.step();
					ppu.step(); ppu.step(); ppu.step();
				}
			}
			//std::cout << std::hex << (int)cpu.PC << std::endl;

		}
		
		display.renderizar();
		
	}
	//salvarArquivo(); // PARA DEBUG CPU
	display.destroy();
	delete bus;
	return 0;
}
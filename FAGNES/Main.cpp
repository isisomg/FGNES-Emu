#include "CPU.h"
#include "Bus.h"
#include "SDL_Display.h"
#include "Cartucho.h"
#include <sstream> // PARA DEBUG APENAS, COMENTAR CASO NAO FOR DEBUGAR
#include <iomanip> // PARA DEBUG APENAS, COMENTAR CASO NAO FOR DEBUGAR
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

// PARA DEBUGAR CPU. DEIXAR COMENTADO CASO NAO FOR DEBUGAR.
std::vector<std::string> info;
void salvarArquivo() { // cria a saida de resultados obtidos da cpu
	std::string path = "resultadosObtidos.txt";
	std::ofstream arq(path, std::ios::out);
	if (arq.is_open() == false) {
		std::cout << "Erro ao gerar arquivo de resultados obtidos" << std::endl;
		return;
	}
	std::string dados = "";
	for (int i = 0; i < info.size(); i++) {
		dados += info.at(i);
	}
	arq << dados;
	arq.close();
}
void guardarLinha(DWord PC, Byte A, Byte X, Byte Y, Byte P, Byte SP) {
	std::stringstream ss;
	ss << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << (int)PC;
	ss << " A:";
	ss << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (int)A;
	ss << " X:";
	ss << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (int)X;
	ss << " Y:";
	ss << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (int)Y;
	ss << " P:";
	ss << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (int)P;
	ss << " SP:";
	ss << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (int)SP;
	ss << std::endl;
	info.push_back(ss.str());
}
// ATE AQUI COMENTADO CASO NAO FOR DEBUGAR.

int main(int argc, char* argv[]) {

	Bus* bus = new Bus();
	Cartucho cartucho;

	SDL_Display display;
	display.init(bus, &cartucho);
	
	CPU cpu;
	PPU ppu;

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
			}

			if (cpu.PC == 0xFFFF) {
				break;
			}
			//guardarLinha(cpu.PC, cpu.A, cpu.X, cpu.Y, cpu.getStatusRegister(), cpu.SP); // PARA DEBUNG CPU
			for (int i = 0; i < 27384; ++i) {
				ppu.step();
			}
			std::cout << std::hex << (int)cpu.PC << std::endl;
			cpu.executar();
	

			//cpu.writeByte(0x00FE, rand() % 0xFF); // mudar RNG. USADO APENAS NA SNAKE PARA TESTE
			//std::cout << std::hex << (int)cpu.PC << std::endl;
			//system("pause");
		}
		
		display.renderizar();
		
	}
	//salvarArquivo(); // PARA DEBUG CPU
	display.destroy();
	delete bus;
	return 0;
}
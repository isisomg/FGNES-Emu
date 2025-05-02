#include <SDL.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

#include "CPU.cpp"
#include "Memoria.cpp"

int TELA_HEIGHT = 32;
int TELA_WIDTH = 32;
int SCALE = 8;

int FPS = 60;

struct Pixel {
	Uint8 r, g, b;
};

Pixel cores[] = {
		{0x7C,0x7C,0x7C},
		{0x00,0x00,0xFC},
		{0x00,0x00,0xBC},
		{0x44,0x28,0xBC},
		{0x94,0x00,0x84},
		{0xA8,0x00,0x20},
		{0xA8,0x10,0x00},
		{0x88,0x14,0x00},
		{0x50,0x30,0x00},
		{0x00,0x78,0x00},
		{0x00,0x68,0x00},
		{0x00,0x58,0x00},
		{0x00,0x40,0x58},
		{0x00,0x00,0x00},
		{0x00,0x00,0x00},
		{0x00,0x00,0x00},
		{0xBC,0xBC,0xBC},
		{0x00,0x78,0xF8},
		{0x00,0x58,0xF8},
		{0x68,0x44,0xFC},
		{0xD8,0x00,0xCC},
		{0xE4,0x00,0x58},
		{0xF8,0x38,0x00},
		{0xE4,0x5C,0x10},
		{0xAC,0x7C,0x00},
		{0x00,0xB8,0x00},
		{0x00,0xA8,0x00},
		{0x00,0xA8,0x44},
		{0x00,0x88,0x88},
		{0x00,0x00,0x00},
		{0x00,0x00,0x00},
		{0x00,0x00,0x00},
		{0xF8,0xF8,0xF8},
		{0x3C,0xBC,0xFC},
		{0x68,0x88,0xFC},
		{0x98,0x78,0xF8},
		{0xF8,0x78,0xF8},
		{0xF8,0x58,0x98},
		{0xF8,0x78,0x58},
		{0xFC,0xA0,0x44},
		{0xF8,0xB8,0x00},
		{0xB8,0xF8,0x18},
		{0x58,0xD8,0x54},
		{0x58,0xF8,0x98},
		{0x00,0xE8,0xD8},
		{0x78,0x78,0x78},
		{0x00,0x00,0x00},
		{0x00,0x00,0x00},
		{0xFC,0xFC,0xFC},
		{0xA4,0xE4,0xFC},
		{0xB8,0xB8,0xF8},
		{0xD8,0xB8,0xF8},
		{0xF8,0xB8,0xF8},
		{0xF8,0xA4,0xC0},
		{0xF0,0xD0,0xB0},
		{0xFC,0xE0,0xA8},
		{0xF8,0xD8,0x78},
		{0xD8,0xF8,0x78},
		{0xB8,0xF8,0xB8},
		{0xB8,0xF8,0xD8},
		{0x00,0xFC,0xFC},
		{0xF8,0xD8,0xF8},
		{0x00,0x00,0x00},
		{0x00,0x00,0x00}
};

int main(int argc, char* argv[]) {

	Memoria mem;
	CPU cpu;
	cpu.inicializar();

	// Carrega os dados na memoria

	std::string path = "snake.hex";
	std::ifstream input(path, std::ios::in | std::ios::binary);
	if (input.is_open() == false) {
		return -1;
	}

	input.seekg(0, input.end);
	int tamanho = input.tellg();
	input.seekg(0, input.beg);

	if (cpu.PC + tamanho >= 65536) {
		std::cerr << "Arquivo muito grande para a memória!" << std::endl;
		return -1;
	}

	for (int i = 0; i < tamanho; i++) {
		input.read((char*)&mem[cpu.PC + i], sizeof(char));
	}

	printf("\n");

	// Dados carregados na memoria com sucesso

	//Inicializando o SDL
	SDL_Init(SDL_INIT_VIDEO); //Por enqt nao temos audio, ent o pra economizar memoria vamos usar apenas SDL_INIT_VIDEO, no futuro podemos trocar por SDL_INIT_EVERYTHING

	SDL_Window* window = SDL_CreateWindow(
		"FaGNES", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		TELA_WIDTH * SCALE,			//Multiplicamos por scale para a janela ter uma resolu  o maior, mudar dps pra tela ser sizeable se necess rio.
		TELA_HEIGHT * SCALE,		//Multiplicamos por scale para a janela ter uma resolu  o maior, mudar dps pra tela ser sizeable se necess rio.
		SDL_WINDOW_SHOWN);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	// cria texture com formato ARGB8888, isso utiliza mais memoria que RGB888, o que pode resultar em erros de exibi  o, por m ARGB8888 tem acesso direto com unit32_t*
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, TELA_WIDTH, TELA_HEIGHT);
	
	// Verificando se a janela foi criada corretamente e se o SDL nao pegou fogo ou sla alguma coisa cruel
	if (!window || !renderer || !texture) {
		SDL_Log("Erro BRUTAL ao iniciar o SDL: %s", SDL_GetError());
		return -1;
	}

	bool running = true;
	SDL_Event event;
	std::srand(std::time(nullptr));



	// Loop principal
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) running = false;

			// Função pro controle funcionar


			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_w:
					printf("Botao apertado: w\n");
					mem[0x00FF] = 0x77;
					break;
				case SDLK_d:
					printf("Botao apertado: d\n");
					mem[0x00FF] = 0x64;
					break;
				case SDLK_s:
					printf("Botao apertado: s\n");
					mem[0x00FF] = 0x73;
					break;
				case SDLK_a:
					printf("Botao apertado: a\n");
					mem[0x00FF] = 0x61;
					break;
				}
			}

		}
		
		if (cpu.PC == 0xFFFF) {
			break;
		}
		cpu.executar(mem);

		if (cpu.atualizarGrafico == false) {
			continue;
		}
		cpu.atualizarGrafico = false;

		void* pixels;
		int pitch;
		SDL_LockTexture(texture, nullptr, &pixels, &pitch);
		uint32_t* pixel_ptr = static_cast<uint32_t*>(pixels);

		for (int y = 0; y < TELA_HEIGHT; ++y) {
			for (int x = 0; x < TELA_WIDTH; ++x) {
				int index = y * (pitch / 4) + x;

				int cor_index = mem[0x0200 + index];
				Pixel cor = cores[cor_index];

				pixel_ptr[index] = (255 << 24) | (cor.r << 16) | (cor.g << 8) | cor.b;
			}
		}
		SDL_UnlockTexture(texture);


		SDL_RenderClear(renderer);

		SDL_Rect dstRect = { 0, 0, TELA_WIDTH * SCALE, TELA_HEIGHT * SCALE };
		SDL_RenderCopy(renderer, texture, nullptr, &dstRect);

		SDL_RenderPresent(renderer);
		SDL_Delay(16);
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;

}
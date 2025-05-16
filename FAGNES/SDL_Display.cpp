#pragma once
#include "SDL_Display.h"
#include "AbrirRom.h"

void SDL_Display::init(Bus* novoBus, Cartucho* cartuchoNovo) {
	cartucho = cartuchoNovo;
	bus = novoBus;
	SDL_Init(SDL_INIT_VIDEO);
	WINDOW = SDL_CreateWindow(
		"FaGNES", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		TELA_WIDTH * ZOOM,			//Multiplicamos por scale para a janela ter uma resolu  o maior, mudar dps pra tela ser sizeable se necess rio.
		TELA_HEIGHT * ZOOM,		//Multiplicamos por scale para a janela ter uma resolu  o maior, mudar dps pra tela ser sizeable se necess rio.
		SDL_WINDOW_SHOWN);

	RENDERER = SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_ACCELERATED);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplSDL2_InitForSDLRenderer(WINDOW, RENDERER);
	ImGui_ImplSDLRenderer2_Init(RENDERER);


	

	TEXTURE = SDL_CreateTexture(RENDERER, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, TELA_WIDTH, TELA_HEIGHT);

	if (!WINDOW) {
		SDL_Log("Erro ao criar janela: %s", SDL_GetError());
		exit(1);
	}
	if (!RENDERER) {
		SDL_Log("Erro ao criar renderer: %s", SDL_GetError());
		exit(1);
	}
	if (!TEXTURE) {
		SDL_Log("Erro ao criar textura: %s", SDL_GetError());
		exit(1);
	}
}

void SDL_Display::processarEntrada(SDL_Event event) {
	if (!bus) return;
	Controles* controles = bus->getControles(); //pega o ponteiro pros controles

	if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
		bool pressionado = (event.type == SDL_KEYDOWN);

		switch (event.key.keysym.sym) {
		case SDLK_z:
			pressionado ? controles->pressionar(A) : controles->soltar(A);
			break;
		case SDLK_x:
			pressionado ? controles->pressionar(B) : controles->soltar(B);
			break;
		case SDLK_d:
			pressionado ? controles->pressionar(START) : controles->soltar(START);
			break;
		case SDLK_f:
			pressionado ? controles->pressionar(SELECT) : controles->soltar(SELECT);
			break;
		case SDLK_UP:
			pressionado ? controles->pressionar(UP) : controles->soltar(UP);
			bus->write(0x00FF, 0x77); // para o snake
			break;
		case SDLK_DOWN:
			pressionado ? controles->pressionar(DOWN) : controles->soltar(DOWN);
			bus->write(0x00FF, 0x73); // para o snake
			break;
		case SDLK_LEFT:
			pressionado ? controles->pressionar(LEFT) : controles->soltar(LEFT);
			bus->write(0x00FF, 0x61); // para o snake
			break;
		case SDLK_RIGHT:
			pressionado ? controles->pressionar(RIGHT) : controles->soltar(RIGHT);
			bus->write(0x00FF, 0x64); // Para o snake
			break;
		}
	}
}

void SDL_Display::renderizar() {
	void* pixels;
	int pitch;
	SDL_LockTexture(TEXTURE, nullptr, &pixels, &pitch);
	uint32_t* pixel_ptr = static_cast<uint32_t*>(pixels);

	// Ajustar para fazer a renderizacao da PPU corretamente
	// APENAS PARA TESTE DO SNAKE
	for (int y = 0; y < TELA_HEIGHT; ++y) {
		for (int x = 0; x < TELA_WIDTH; ++x) {
			int index = y * (pitch / 4) + x;

			int cor_index = bus->read(0x0200 + index);
			Pixel cor = cores[cor_index];

			pixel_ptr[index] = (255 << 24) | (cor.r << 16) | (cor.g << 8) | cor.b;
		}
	}

	SDL_UnlockTexture(TEXTURE);

	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	// Define a posição e o tamanho da janela fixa no topo
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2((float)TELA_WIDTH * ZOOM, 50)); // Altura de 50 pixels para o menu

	// Flags para manter a janela fixa e não redimensionável
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

	ImGui::Begin("Menu", nullptr, flags);
	if (ImGui::Button("Abrir")) {
		std::string arquivoROM = AbrirArquivo();
		cartucho->init(arquivoROM);
		jogoRodando = true;
	}
	ImGui::End();
	ImGui::Render();

	SDL_RenderClear(RENDERER);

	SDL_Rect dstRect = { 0, 0, TELA_WIDTH * ZOOM, TELA_HEIGHT * ZOOM };
	SDL_RenderCopy(RENDERER, TEXTURE, nullptr, &dstRect);
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), RENDERER);

	SDL_RenderPresent(RENDERER);
	SDL_Delay(32);
}

void SDL_Display::destroy() {
	// Shutdown do ImGui
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	// Shutdown do SDL
	SDL_DestroyTexture(TEXTURE);
	SDL_DestroyRenderer(RENDERER);
	SDL_DestroyWindow(WINDOW);
	SDL_Quit();
}

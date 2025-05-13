#pragma once
#include "SDL_Display.h"

void SDL_Display::init(Bus* novoBus) {
	bus = novoBus;
	SDL_Init(SDL_INIT_VIDEO);
	WINDOW = SDL_CreateWindow(
		"FaGNES", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		TELA_WIDTH * ZOOM,			//Multiplicamos por scale para a janela ter uma resolu  o maior, mudar dps pra tela ser sizeable se necess rio.
		TELA_HEIGHT * ZOOM,		//Multiplicamos por scale para a janela ter uma resolu  o maior, mudar dps pra tela ser sizeable se necess rio.
		SDL_WINDOW_SHOWN);

	RENDERER = SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_ACCELERATED);
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
	if (event.type == SDL_KEYDOWN) {
		switch (event.key.keysym.sym) {
		case SDLK_w:
			printf("Botao apertado: w\n");
			bus->write(0x00FF, 0x77);
			break;
		case SDLK_d:
			printf("Botao apertado: d\n");
			bus->write(0x00FF, 0x64);
			break;
		case SDLK_s:
			printf("Botao apertado: s\n");
			bus->write(0x00FF, 0x73);
			break;
		case SDLK_a:
			printf("Botao apertado: a\n");
			bus->write(0x00FF, 0x61);
			break;
		}
	}
}

void SDL_Display::renderizar() {
	void* pixels;
	int pitch;
	SDL_LockTexture(TEXTURE, nullptr, &pixels, &pitch);
	uint32_t* pixel_ptr = static_cast<uint32_t*>(pixels);

	for (int y = 0; y < TELA_HEIGHT; ++y) {
		for (int x = 0; x < TELA_WIDTH; ++x) {
			int index = y * (pitch / 4) + x;

			int cor_index = bus->read(0x0200 + index);
			Pixel cor = cores[cor_index];

			pixel_ptr[index] = (255 << 24) | (cor.r << 16) | (cor.g << 8) | cor.b;
		}
	}
	SDL_UnlockTexture(TEXTURE);


	SDL_RenderClear(RENDERER);

	SDL_Rect dstRect = { 0, 0, TELA_WIDTH * ZOOM, TELA_HEIGHT * ZOOM };
	SDL_RenderCopy(RENDERER, TEXTURE, nullptr, &dstRect);

	SDL_RenderPresent(RENDERER);
	SDL_Delay(32);
}

void SDL_Display::destroy() {
	SDL_DestroyTexture(TEXTURE);
	SDL_DestroyRenderer(RENDERER);
	SDL_DestroyWindow(WINDOW);
	SDL_Quit();
}

//// MAIN FALSO APENAS PARA PODER TESTAR O AUDIO
//#include <SDL.h>
//#include <iostream>
//
//class SDL_Teste {
//private:
//    const int TELA_WIDTH = 32;
//    const int TELA_HEIGHT = 32;
//    int ZOOM = 8;
//
//    SDL_Texture* TEXTURE;
//    SDL_Window* WINDOW;
//
//
//public:
//    bool jogoRodando = false;
//
//    SDL_Renderer* RENDERER;
//
//	void init() {
//		SDL_Init(SDL_INIT_VIDEO);
//		WINDOW = SDL_CreateWindow(
//			"FaGNES", SDL_WINDOWPOS_CENTERED,
//			SDL_WINDOWPOS_CENTERED,
//			TELA_WIDTH * ZOOM,			//Multiplicamos por scale para a janela ter uma resolu  o maior, mudar dps pra tela ser sizeable se necess rio.
//			TELA_HEIGHT * ZOOM,		//Multiplicamos por scale para a janela ter uma resolu  o maior, mudar dps pra tela ser sizeable se necess rio.
//			SDL_WINDOW_SHOWN);
//
//		RENDERER = SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_ACCELERATED);
//
//		TEXTURE = SDL_CreateTexture(RENDERER, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, TELA_WIDTH, TELA_HEIGHT);
//
//		if (!WINDOW) {
//			SDL_Log("Erro ao criar janela: %s", SDL_GetError());
//			exit(1);
//		}
//		if (!RENDERER) {
//			SDL_Log("Erro ao criar renderer: %s", SDL_GetError());
//			exit(1);
//		}
//		if (!TEXTURE) {
//			SDL_Log("Erro ao criar textura: %s", SDL_GetError());
//			exit(1);
//		}
//	}
//    void renderizar() {
//		void* pixels;
//		int pitch;
//		SDL_LockTexture(TEXTURE, nullptr, &pixels, &pitch);
//		uint32_t* pixel_ptr = static_cast<uint32_t*>(pixels);
//
//		for (int y = 0; y < TELA_HEIGHT; ++y) {
//			for (int x = 0; x < TELA_WIDTH; ++x) {
//				int index = y * (pitch / 4) + x;
//
//				pixel_ptr[index] = (255 << 24) | (0 << 16) | (0 << 8) | 0;
//			}
//		}
//
//		SDL_UnlockTexture(TEXTURE);
//
//		SDL_RenderClear(RENDERER);
//
//		SDL_Rect dstRect = { 0, 0, TELA_WIDTH * ZOOM, TELA_HEIGHT * ZOOM };
//		SDL_RenderCopy(RENDERER, TEXTURE, nullptr, &dstRect);
//
//		SDL_RenderPresent(RENDERER);
//		SDL_Delay(32);
//	}
//	void destroy() {
//		SDL_DestroyTexture(TEXTURE);
//		SDL_DestroyRenderer(RENDERER);
//		SDL_DestroyWindow(WINDOW);
//		SDL_Quit();
//	}
//
//	void processarEntrada(SDL_Event event) {
//		if (event.type == SDL_KEYDOWN) {
//			switch (event.key.keysym.sym) {
//			case SDLK_w:
//				printf("Botao apertado: w\n");
//				break;
//			case SDLK_d:
//				printf("Botao apertado: d\n");
//				break;
//			case SDLK_s:
//				printf("Botao apertado: s\n");
//				break;
//			case SDLK_a:
//				printf("Botao apertado: a\n");
//				break;
//			}
//		}
//	}
//    //~SDL_Display();
//};
//
//int main(int argc, char* argv[]) {
//	SDL_Teste display;
//	display.init();
//
//	bool rodar = true;
//	SDL_Event event;
//	while (rodar) {
//		while (SDL_PollEvent(&event)) {
//			if (event.type == SDL_QUIT) rodar = false;
//			display.processarEntrada(event);
//		}
//		
//
//		display.renderizar();
//	}
//	return 0;
//}
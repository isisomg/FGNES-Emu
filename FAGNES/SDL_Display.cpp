	#pragma once
	#include "SDL_Display.h"
	#include "AbrirRom.h"

	void SDL_Display::init(Bus* novoBus, Cartucho* cartuchoNovo, PPU* p) {
		cartucho = cartuchoNovo;
		bus = novoBus;
		ppu = p;
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
		//// 1. Criar e configurar APU
		//apu = new APU();
		//apu->setFrequency(44100.0f, 4); // 4 canais
		//apu->setEnabled(true);

		//// 2. Passar APU para o Bus
		//novoBus->setAPU(apu);

		//// 3. Inicializar �udio SDL
		//SDL_AudioSpec desiredSpec;
		//SDL_zero(desiredSpec);
		//desiredSpec.freq = 44100;
		//desiredSpec.format = AUDIO_F32SYS;
		//desiredSpec.channels = 1;
		//desiredSpec.samples = 512;
		//desiredSpec.callback = audioCallback;
		//desiredSpec.userdata = this;

		//audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, nullptr, 0);
		//if (audioDevice == 0) {
		//	SDL_Log("Erro ao abrir dispositivo de audio: %s", SDL_GetError());
		//	exit(1);
		//}

		//SDL_PauseAudioDevice(audioDevice, 0);
	}

	void SDL_Display::processarEntrada(SDL_Event event) {
		if (!bus) return;
		Controles* controles = bus->getControles(); //pega o ponteiro pros controles

		if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
			bool pressionado = (event.type == SDL_KEYDOWN);

			switch (event.key.keysym.sym) {
				case SDLK_z:
					pressionado ? controles->pressionar(A) : controles->soltar(A);
					std::cout << "APERTOU A TECLA Z" << std::endl;
					break;
				case SDLK_x:
					pressionado ? controles->pressionar(B) : controles->soltar(B);
					std::cout << "APERTOU A TECLA X" << std::endl;
					break;
				case SDLK_d:
					pressionado ? controles->pressionar(START) : controles->soltar(START);
					std::cout << "APERTOU A TECLA START" << std::endl;
					break;
				case SDLK_f:
					pressionado ? controles->pressionar(SELECT) : controles->soltar(SELECT);
					std::cout << "APERTOU A TECLA SELECT" << std::endl;
					break;
				case SDLK_UP:
					pressionado ? controles->pressionar(UP) : controles->soltar(UP);
					std::cout << "APERTOU A TECLA UP" << std::endl;
					break;
				case SDLK_DOWN:
					pressionado ? controles->pressionar(DOWN) : controles->soltar(DOWN);
					std::cout << "APERTOU A TECLA DOWN" << std::endl;
					break;
				case SDLK_LEFT:
					pressionado ? controles->pressionar(LEFT) : controles->soltar(LEFT);
					std::cout << "APERTOU A TECLA LEFT" << std::endl;
					break;
				case SDLK_RIGHT:
					pressionado ? controles->pressionar(RIGHT) : controles->soltar(RIGHT);
					std::cout << "APERTOU A TECLA RIGHT" << std::endl;
					break;
				}
			}
		}

	void SDL_Display::renderizar() {
		void* pixels;
		int pitch;
		SDL_LockTexture(TEXTURE, nullptr, &pixels, &pitch);
		uint32_t* pixel_ptr = static_cast<uint32_t*>(pixels);

		//for (int y = 0; y < 240; ++y) { 
		//	memcpy(&pixel_ptr[y * (pitch / 4)], &ppu->framebuffer[y * 256], 256 * sizeof(uint32_t));
		//}
		memcpy(pixel_ptr, ppu->framebuffer, TELA_WIDTH * TELA_HEIGHT * sizeof(uint32_t));// copia o framebuffer para gerar textura


		// LIMPAR FRAMEBUFFER PARA NAO FICAR FUDIDO DE FEIO
		memset(ppu->framebuffer, 0, TELA_WIDTH * TELA_HEIGHT * sizeof(ppu->framebuffer[0]));


		SDL_UnlockTexture(TEXTURE);

		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Arquivo")) {
				if (ImGui::MenuItem("Carregar")) {
					std::string arquivoROM = AbrirArquivo();
					if (!arquivoROM.empty()) {
						cartucho->init(arquivoROM);
						inicializarAudio();
						jogoRodando = true;
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(u8"Op��es")) {
				if (ImGui::MenuItem("Controle")) {
					mostrarJanelaControle = true;
					}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Zoom")) {
				bool enabled = !isFull;

				if (ImGui::MenuItem("1X", nullptr, (ZOOM == 1 && !isFull), enabled)) {
					ZOOM = 1;
					SDL_SetWindowSize(WINDOW, TELA_WIDTH * ZOOM, TELA_HEIGHT * ZOOM);
				}
				if (ImGui::MenuItem("2X", nullptr, (ZOOM == 2 && !isFull), enabled)) {
					ZOOM = 2;
					SDL_SetWindowSize(WINDOW, TELA_WIDTH * ZOOM, TELA_HEIGHT * ZOOM);
				}
				if (ImGui::MenuItem("4X", nullptr, (ZOOM == 4 && !isFull), enabled)) {
					ZOOM = 4;
					SDL_SetWindowSize(WINDOW, TELA_WIDTH * ZOOM, TELA_HEIGHT * ZOOM);
				}
				if (ImGui::MenuItem("Tela Cheia", nullptr, isFull)) {
					isFull = !isFull;
					if (isFull) {
						prevZOOM = ZOOM;                            
						ZOOM = 4;                                   
						SDL_SetWindowFullscreen(WINDOW, SDL_WINDOW_FULLSCREEN_DESKTOP);
					}
					else {
						SDL_SetWindowFullscreen(WINDOW, 0);
						ZOOM = prevZOOM;                            
						SDL_SetWindowSize(
							WINDOW,
							TELA_WIDTH * ZOOM,
							TELA_HEIGHT * ZOOM
						);
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Ajuda")) {
				if (ImGui::MenuItem("Sobre")) {
					SDL_OpenURL("https://github.com/isisomg/FGNES-Emu");
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar(); // FINAL DO MENU

			// Janela de mapeamento de controle
			if (mostrarJanelaControle) {
				// Define a posi��o e tamanho iniciais fora da �rea da tela do emulador
				ImGui::SetNextWindowPos(ImVec2(500, 100), ImGuiCond_FirstUseEver); // Altere conforme necess�rio
				ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);

				ImGui::Begin("Mapeamento de Controle NES", &mostrarJanelaControle,
					ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

				static const char* nomesBotoesNES[8] = {
					"Cima", "Baixo", "Esquerda", "Direita", "B", "A", "Start", "Select"
				};

				static SDL_Scancode mapeamentos[8] = {
					SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
					SDL_SCANCODE_X, SDL_SCANCODE_Z, SDL_SCANCODE_D, SDL_SCANCODE_F
				};

				static int aguardandoBotao = -1;

				for (int i = 0; i < 8; ++i) {
					ImGui::Text("%s:", nomesBotoesNES[i]);
					ImGui::SameLine();

					char label[32];
					snprintf(label, sizeof(label), "Mapear##%d", i);
					if (ImGui::Button(label)) {
						aguardandoBotao = i;
					}
					ImGui::SameLine();
					ImGui::Text("%s", SDL_GetScancodeName(mapeamentos[i]));
				}
				if (aguardandoBotao != -1) {
					ImGui::Text(u8"Pressione uma tecla para o bot�o: %s", nomesBotoesNES[aguardandoBotao]);

					const Uint8* state = SDL_GetKeyboardState(nullptr);
					for (int sc = 0; sc < SDL_NUM_SCANCODES; ++sc) {
						if (state[sc]) {
							mapeamentos[aguardandoBotao] = static_cast<SDL_Scancode>(sc);
							aguardandoBotao = -1;
							break;
						}
					}
				}
				ImGui::End();
			}
		}
		ImGui::Render();

		SDL_RenderClear(RENDERER);

		/*SDL_Rect dstRect = { 0, 0, TELA_WIDTH * ZOOM, TELA_HEIGHT * ZOOM };*/
		// Antes de criar o dstRect, pegue o tamanho atual da janela:
		int winW, winH;
		SDL_GetWindowSize(WINDOW, &winW, &winH);

		int dstW = TELA_WIDTH * ZOOM;
		int dstH = TELA_HEIGHT * ZOOM;

		int offsetX;
		int offsetY;
		if (isFull) {
			// centraliza horizontal e verticalmente
			offsetX = (winW - dstW) / 2;
			offsetY = (winH - dstH) / 2;
		}
		else {
			// mant�m o menu bar no topo quando n�o � fullscreen
			float menuBarAltura = ImGui::GetFrameHeight();
			offsetX = 0;
			offsetY = static_cast<int>(menuBarAltura);
		}

		SDL_Rect dstRect = { offsetX, offsetY, dstW, dstH };
		SDL_RenderCopy(RENDERER, TEXTURE, nullptr, &dstRect);


		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), RENDERER);

		SDL_RenderPresent(RENDERER);

		SDL_Delay(16);
	}

	void SDL_Display::destroy() {
		 //minhas coisinhassss
		if (audioDevice != 0) {
			SDL_CloseAudioDevice(audioDevice);
			audioDevice = 0;
		}

		if (apu) {
			delete apu;
			apu = nullptr;
		}
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
	// tudo meu: cyro
	void SDL_Display::audioCallback(void* userdata, Uint8* stream, int len) {
		SDL_Display* self = static_cast<SDL_Display*>(userdata);
		if (!self || !self->apu) return;

		float* buffer = reinterpret_cast<float*>(stream);
		int samples = len / sizeof(float);

		for (int i = 0; i < samples; i++) {
			//self->apu->step(); // avan�a todos os canais
			//buffer[i] = self->apu->getMixedSample(); // retorna o som combinado dos canais
			const float cyclesPerSample = 1789773.0f / 44100.0f;
			self->apu->stepCpuCycles(cyclesPerSample);
			buffer[i] = self->apu->getMixedSample();
		}
	}

	void SDL_Display::inicializarAudio() {
		// Fecha o dispositivo de �udio anterior, se existir
		if (audioDevice != 0) {
			SDL_CloseAudioDevice(audioDevice);
			audioDevice = 0;
		}
		// Libera a APU anterior, se existir
		if (apu) {
			delete apu;
			apu = nullptr;
		}
		// Cria nova APU e configura
		apu = new APU();
		//apu->setFrequency(44100.0f, 4);
		apu->setEnabled(true);
		bus->setAPU(apu);

		SDL_AudioSpec desiredSpec;
		SDL_zero(desiredSpec);
		desiredSpec.freq = 44100;
		desiredSpec.format = AUDIO_F32SYS;
		desiredSpec.channels = 1;
		desiredSpec.samples = 512;
		desiredSpec.callback = audioCallback;
		desiredSpec.userdata = this;

		audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, nullptr, 0);
		if (audioDevice == 0) {
			SDL_Log("Erro ao abrir dispositivo de audio: %s", SDL_GetError());
		}
		else {
			SDL_PauseAudioDevice(audioDevice, 0);
		}
	}
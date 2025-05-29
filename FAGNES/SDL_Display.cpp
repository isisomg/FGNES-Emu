#pragma once
#include "SDL_Display.h"
#include "AbrirRom.h"
#include "Controles.h"


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
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // permite que as janelas possam sair do main viewport do sdl
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

	//// 3. Inicializar áudio SDL
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
	ImGuiIO& io = ImGui::GetIO();

	if (event.type == SDL_KEYDOWN && mostrarJanelaControle && botaoAguardandoMapeamento != -1) {
		SDL_Scancode teclaPressionada = event.key.keysym.scancode;

		if (teclaPressionada == SDL_SCANCODE_ESCAPE) {
			botaoAguardandoMapeamento = -1;
		}
		else if (teclaPressionada != SDL_SCANCODE_UNKNOWN) {
			botoesNES botaoParaMapear = static_cast<botoesNES>(botaoAguardandoMapeamento);
			botoesNES botaoEmConflito;

			if (controles->setScancodeParaBotao(botaoParaMapear, teclaPressionada, &botaoEmConflito)) {
			}
			else {

				this->mensagemPopupTeclaEmUso = "A tecla '";
				this->mensagemPopupTeclaEmUso += SDL_GetScancodeName(teclaPressionada);
				this->mensagemPopupTeclaEmUso += "' ja esta em uso pelo botao '";
				this->mensagemPopupTeclaEmUso += botaoParaString(botaoEmConflito);
				this->mensagemPopupTeclaEmUso += "'.\n\nEscolha outra tecla ou desmapeie o botao '";
				this->mensagemPopupTeclaEmUso += botaoParaString(botaoEmConflito);
				this->mensagemPopupTeclaEmUso += "' primeiro.";
				this->mostrarPopupTeclaEmUso = true;
			}
			botaoAguardandoMapeamento = -1;
		}
		return;
	}

	/*if (event.type == SDL_KEYDOWN && mostrarJanelaControle && botaoAguardandoMapeamento != -1) {
		if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			botaoAguardandoMapeamento = -1;
			std::cout << "Mapeamento para o botao cancelado com ESC." << std::endl;
		}
		else if (event.key.keysym.scancode != SDL_SCANCODE_UNKNOWN) {
			controles->setScancodeParaBotao(static_cast<botoesNES>(botaoAguardandoMapeamento), event.key.keysym.scancode);
			botaoAguardandoMapeamento = -1;
		}
		return;
	}*/


	// ImGui_ImplSDL2_ProcessEvent(&event); // Deixe para o loop principal em main.cpp para ImGui

	if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
		ImGuiIO& io = ImGui::GetIO(); //Se ImGui quiser o teclado ele prioriza as entradas apenas para a ImGUi

		if (!io.WantCaptureKeyboard) { //precessa apenas se a ImGui não quiser usar o teclado

			bool pressionado = (event.type == SDL_KEYDOWN);

			controles->processarEntrada(event.key.keysym.scancode, pressionado); //Não rodar junto com o switch case abaixo
		}

	}

	/*switch (event.key.keysym.sym) {
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
	}*/

}

void SDL_Display::CallbackLogin(const firebase::Future<firebase::auth::AuthResult>* future, void* user_data) {
	SDL_Display* self = static_cast<SDL_Display*>(user_data);
	if (future->error() == firebase::auth::kAuthErrorNone) {
		const firebase::auth::User* user = &future->result()->user;
		if (user) {
			self->usuarioLogado = true;
			self->nomeUsuarioLogado = std::string(user->email().c_str());
			self->estadoAtualConta = EstadoConta::LoginSucesso;
		}

		std::cout << "Login Firebase bem-sucedido!\n";
		self->usuarioLogado = true;
		self->nomeUsuarioLogado = std::string(user->email().c_str());
		self->estadoAtualConta = EstadoConta::LoginSucesso;
	}
	else {
		self->mensagemErroLogin = future->error_message();
		self->exibirMensagemErroLogin = true;
		self->usuarioLogado = false;
	}
}

void SDL_Display::CallbackCadastro(const firebase::Future<firebase::auth::AuthResult>* future, void* user_data) {
	SDL_Display* self = static_cast<SDL_Display*>(user_data);
	if (future->error() == firebase::auth::kAuthErrorNone) {
		const firebase::auth::User* user = &future->result()->user;
		if (user) {
			self->usuarioLogado = true;
			self->nomeUsuarioLogado = std::string(user->email().c_str());
			self->estadoAtualConta = EstadoConta::LoginSucesso;
		}

		std::cout << "Cadastro Firebase bem-sucedido!\n";
		self->usuarioLogado = true;
		self->nomeUsuarioLogado = std::string(user->email().c_str());
		self->estadoAtualConta = EstadoConta::LoginSucesso;
		self->exibirErroCadastro = false;
	}
	else {
		self->erroCadastro = future->error_message();
		self->exibirErroCadastro = true;
	}
}

void SDL_Display::renderizar() {
	static char popupMessage[256] = ""; // Buffer para mensagens de feedback dos popup
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

		if (ImGui::BeginMenu(u8"Opções")) {
			if (ImGui::MenuItem("Controle")) {
				mostrarJanelaControle = !mostrarJanelaControle;
				if (!mostrarJanelaControle) {
					botaoAguardandoMapeamento = -1;
				}
			}
			if (ImGui::MenuItem("Conta")) {
				mostrarJanelaConta = true;
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

		// Janela de conta
		if (mostrarJanelaConta) {
			static char usernameBuffer[128] = "";
			static char passwordBuffer[128] = "";
			static bool exibirMensagemErroLogin = false;
			static std::string mensagemErroLogin = "";

			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			std::string tituloJanelaConta = u8"Conta"; // Título padrão

			// Se o usuário já está logado, e a UI está no estado inicial de login,
			// pulamos direto para uma UI de "Logado" simplificada (apenas botão de logout).
			// Se não, processamos os estados da UI de login.
			if (usuarioLogado && estadoAtualConta == EstadoConta::FazendoLogin) {
				tituloJanelaConta = u8"Minha Conta";
				ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Appearing); // Pode ser menor
				if (ImGui::Begin(tituloJanelaConta.c_str(), &mostrarJanelaConta, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text(u8"Conectado como: %s", nomeUsuarioLogado.c_str());
					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
					if (ImGui::Button("Logout", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
						std::cout << "Usuário " << nomeUsuarioLogado << " desconectado." << std::endl;
						usuarioLogado = false;
						nomeUsuarioLogado = "";
						estadoAtualConta = EstadoConta::FazendoLogin; // Prepara para próximo login
						// Não precisa fechar a janela aqui, o usuário pode querer logar novamente.
						// Se quiser fechar ao fazer logout: mostrarJanelaConta = false;
					}
					ImGui::End();
				}
				else {
					// Se fechou a janela no X enquanto estava logado, resetar estado da UI para próximo login
					if (!mostrarJanelaConta) {
						estadoAtualConta = EstadoConta::FazendoLogin;
					}
				}

			}
			else { // Processo de login ou tela de sucesso
				if (estadoAtualConta == EstadoConta::FazendoLogin) {
					tituloJanelaConta = u8"Login da Conta";
				}
				else if (estadoAtualConta == EstadoConta::LoginSucesso) {
					tituloJanelaConta = u8"Sucesso!";
				}
				ImGui::SetNextWindowSize(ImVec2(350, 0), ImGuiCond_Appearing);

				if (ImGui::Begin(tituloJanelaConta.c_str(), &mostrarJanelaConta, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
					switch (estadoAtualConta) {
					case EstadoConta::FazendoLogin:
						ImGui::Text(u8"Bem-vindo! Por favor, faça o login.");
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Text(u8"Nome de Usuário:");
						ImGui::SetNextItemWidth(-FLT_MIN);
						ImGui::InputText("##NomeUsuarioLogin", usernameBuffer, IM_ARRAYSIZE(usernameBuffer));
						ImGui::Spacing();

						ImGui::Text("Senha:");
						ImGui::SetNextItemWidth(-FLT_MIN);
						ImGui::InputText("##SenhaLogin", passwordBuffer, IM_ARRAYSIZE(passwordBuffer), ImGuiInputTextFlags_Password);
						ImGui::Spacing();

						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.5f, 1.0f, 1.0f));
						if (ImGui::Selectable("Cadastrar-se")) {
							estadoAtualConta = EstadoConta::Cadastro;
							memset(passwordBuffer, 0, sizeof(passwordBuffer)); // Limpa senha antiga
						}
						ImGui::PopStyleColor();

						ImGui::Separator();
						ImGui::Spacing();

						if (ImGui::Button("Login", ImVec2(ImGui::GetContentRegionAvail().x * 0.48f, 0))) {
							if (firebaseAuth) {
								firebase::Future<firebase::auth::AuthResult> result =
									firebaseAuth->SignInWithEmailAndPassword(usernameBuffer, passwordBuffer);

								result.OnCompletion(
									[](const firebase::Future<firebase::auth::AuthResult>& future, void* user_data) {
										SDL_Display::CallbackLogin(&future, user_data);
									}, this);
							}

						}

						ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.x * 0.5f);

						if (ImGui::Button("Cancelar", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
							mostrarJanelaConta = false;
							// Resetar estado da UI se fechar no meio do login
							estadoAtualConta = EstadoConta::FazendoLogin;
							exibirMensagemErroLogin = false; // Limpa msg de erro
							// Não mexer em usuarioLogado aqui, pois pode estar logado e cancelando uma nova tentativa
							// Limpar buffers apenas se não estiver logado ou se desejar limpar sempre
							if (!usuarioLogado) {
								memset(usernameBuffer, 0, sizeof(usernameBuffer));
							}
							memset(passwordBuffer, 0, sizeof(passwordBuffer));
						}
						ImGui::Spacing();

						if (exibirMensagemErroLogin) {
							ImGui::Separator();
							ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), mensagemErroLogin.c_str());
						}
						break;

					case EstadoConta::LoginSucesso:
						ImGui::Text(u8"Login realizado com sucesso!");
						ImGui::Spacing();
						ImGui::Text(u8"Bem-vindo(a), %s!", nomeUsuarioLogado.c_str());
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();
						if (ImGui::Button("OK", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
							mostrarJanelaConta = false; // Fecha a janela "Conta"
							estadoAtualConta = EstadoConta::FazendoLogin; // Reseta UI para próxima vez
						}
						break;
					case EstadoConta::Cadastro: {
						static char newUser[128] = "";
						static char newPass[128] = "";
						static char confirmPass[128] = "";
						static std::string erroCadastro = "";
						static bool exibirErroCadastro = false;

						ImGui::Text(u8"Nome de Usuário:");
						ImGui::SetNextItemWidth(-FLT_MIN);
						ImGui::InputText("##CadastroUsuario", newUser, IM_ARRAYSIZE(newUser));
						ImGui::Spacing();

						ImGui::Text("Senha:");
						ImGui::SetNextItemWidth(-FLT_MIN);
						ImGui::InputText("##CadastroSenha", newPass, IM_ARRAYSIZE(newPass), ImGuiInputTextFlags_Password);
						ImGui::Spacing();

						ImGui::Text("Confirmar Senha:");
						ImGui::SetNextItemWidth(-FLT_MIN);
						ImGui::InputText("##CadastroConfirmaSenha", confirmPass, IM_ARRAYSIZE(confirmPass), ImGuiInputTextFlags_Password);
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						if (ImGui::Button("Cadastrar", ImVec2(ImGui::GetContentRegionAvail().x * 0.48f, 0))) {
							if (strcmp(newPass, confirmPass) != 0) {
								erroCadastro = u8"As senhas não coincidem.";
								exibirErroCadastro = true;
							}
							else if (firebase::auth::kAuthErrorEmailAlreadyInUse) {
								erroCadastro = u8"E-mail já está em uso.";
								exibirErroCadastro = true;
							}
							else if (strlen(newUser) < 3 || strlen(newPass) < 6) {
								erroCadastro = u8"Usuário precisa de 3+ letras e senha 6+.";
								exibirErroCadastro = true;
							}
							else if (firebaseAuth) {
								firebase::Future<firebase::auth::AuthResult> result =
									firebaseAuth->CreateUserWithEmailAndPassword(newUser, newPass);

								result.OnCompletion(
									[](const firebase::Future<firebase::auth::AuthResult>& future, void* user_data) {
										SDL_Display::CallbackCadastro(&future, user_data);
									}, this);
							}
						}

						ImGui::SameLine();

						if (ImGui::Button("Cancelar", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
							estadoAtualConta = EstadoConta::FazendoLogin;
							exibirErroCadastro = false;
						}

						if (exibirErroCadastro) {
							ImGui::Spacing();
							ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), erroCadastro.c_str());
						}

						break;
					}
					}
					ImGui::End();
				}
				else {
					// Se a janela "Conta" foi fechada pelo 'X', resetamos a UI para o estado de login
					// para a próxima vez que for aberta, independentemente do estado de login do usuário.
					if (!mostrarJanelaConta) {
						estadoAtualConta = EstadoConta::FazendoLogin;
						exibirMensagemErroLogin = false;
						// Não limpar buffers aqui, pois podem ser úteis se o usuário reabrir imediatamente
						// ou limpar se preferir um comportamento mais "resetado":
						// memset(usernameBuffer, 0, sizeof(usernameBuffer));
						// memset(passwordBuffer, 0, sizeof(passwordBuffer));
					}
				}
			}
		}
	}

	// Janela de mapeamento de controle

	if (mostrarJanelaControle && bus && bus->getControles()) {

		Controles* controles = bus->getControles();

		ImGui::SetNextWindowSize(ImVec2(430, 390), ImGuiCond_FirstUseEver); // Ajuste o tamanho conforme necessário
		ImGui::Begin("Mapeamento de Controle NES", &mostrarJanelaControle, ImGuiWindowFlags_NoCollapse);

		if (!mostrarJanelaControle) {
			botaoAguardandoMapeamento = -1;
		}

		ImGui::TextWrapped("Clique em 'Mapear' ao lado de um botao e pressione a tecla desejada.\nPressione ESC para cancelar o mapeamento do botao atual.");
		ImGui::Separator();
		ImGui::Spacing();

		const botoesNES ordemBotoesGui[] = {
			botoesNES::UP, botoesNES::DOWN, botoesNES::LEFT, botoesNES::RIGHT,
			botoesNES::A, botoesNES::B, botoesNES::SELECT, botoesNES::START
		};

		for (botoesNES botaoAtualEnum : ordemBotoesGui) {
			const char* nomeExibicaoBotao = botaoParaString(botaoAtualEnum);
			SDL_Scancode scancodeMapeado = controles->getScancodeParaBotao(botaoAtualEnum);
			const char* nomeTeclaMapeada = (scancodeMapeado != SDL_SCANCODE_UNKNOWN) ? SDL_GetScancodeName(scancodeMapeado) : "N/A";

			ImGui::Text("%s:", nomeExibicaoBotao);
			ImGui::SameLine(150.0f); // Ajuste para alinhar

			if (botaoAguardandoMapeamento == static_cast<int>(botaoAtualEnum)) {
				ImGui::TextDisabled("[Pressione uma tecla... (ESC para cancelar)]");
			}
			else {
				std::string labelBotaoMapear = std::string("Mapear##") + nomeExibicaoBotao;
				if (ImGui::Button(labelBotaoMapear.c_str(), ImVec2(100, 0))) {
					botaoAguardandoMapeamento = static_cast<int>(botaoAtualEnum);
				}
			}
			ImGui::SameLine(270.0f); // Ajuste para alinhar
			ImGui::TextUnformatted(nomeTeclaMapeada);
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Button("Salvar Mapeamentos", ImVec2(160, 0))) {
			if (controles->salvarMapeamento()) {
				strncpy_s(popupMessage, sizeof(popupMessage), "Mapeamentos salvos com sucesso!", _TRUNCATE);
				ImGui::OpenPopup("FeedbackPopup");
			}
			else {
				strncpy_s(popupMessage, sizeof(popupMessage), "ERRO ao salvar mapeamentos.", _TRUNCATE);
				ImGui::OpenPopup("FeedbackPopup");
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Carregar Mapeamentos", ImVec2(160, 0))) {
			botaoAguardandoMapeamento = -1;

			if (controles->carregarMapeamento()) {
				strncpy_s(popupMessage, sizeof(popupMessage), "Mapeamentos carregados com sucesso!", _TRUNCATE);
				ImGui::OpenPopup("FeedbackPopup");
			}
			else {
				strncpy_s(popupMessage, sizeof(popupMessage), "ERRO ao carregar mapeamentos.\n(Verifique se 'controles.json' existe junto ao .exe)", _TRUNCATE);
				ImGui::OpenPopup("FeedbackPopup");
			}
		}

		ImGui::Spacing();

		if (ImGui::Button("Restaurar Padroes", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
			botaoAguardandoMapeamento = -1;
			controles->reverterParaPadrao();

			strncpy_s(popupMessage, sizeof(popupMessage), "Padroes restaurados.\nClique em 'Salvar' para persistir.", _TRUNCATE);
			ImGui::OpenPopup("FeedbackPopup");
		}

		//Popup de feedback genérico
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("FeedbackPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
			ImGui::TextWrapped("%s", popupMessage);
			ImGui::Separator();
			ImGui::Spacing();
			if (ImGui::Button("OK", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::EndPopup();
		}

		ImGui::End();

		if (this->mostrarPopupTeclaEmUso) {
			ImGui::OpenPopup("TeclaJaEmUsoPopup");
		}

		if (ImGui::BeginPopupModal("TeclaJaEmUsoPopup", &this->mostrarPopupTeclaEmUso, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
			ImGui::TextWrapped("%s", this->mensagemPopupTeclaEmUso.c_str());
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			if (ImGui::Button("OK", ImVec2(120, 0))) {
				this->mostrarPopupTeclaEmUso = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::EndPopup();
		}

	}


	//if (mostrarJanelaControle) {
	//	// Define a posição e tamanho iniciais fora da área da tela do emulador
	//	ImGui::SetNextWindowPos(ImVec2(500, 100), ImGuiCond_FirstUseEver); // Altere conforme necessário
	//	ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);

	//	ImGui::Begin("Mapeamento de Controle NES", &mostrarJanelaControle,
	//		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	//	static const char* nomesBotoesNES[8] = {
	//		"Cima", "Baixo", "Esquerda", "Direita", "B", "A", "Start", "Select"
	//	};

	//	static SDL_Scancode mapeamentos[8] = {
	//		SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
	//		SDL_SCANCODE_X, SDL_SCANCODE_Z, SDL_SCANCODE_D, SDL_SCANCODE_F
	//	};

	//	static int aguardandoBotao = -1;

	//	for (int i = 0; i < 8; ++i) {
	//		ImGui::Text("%s:", nomesBotoesNES[i]);
	//		ImGui::SameLine();

	//		char label[32];
	//		snprintf(label, sizeof(label), "Mapear##%d", i);
	//		if (ImGui::Button(label)) {
	//			aguardandoBotao = i;
	//		}
	//		ImGui::SameLine();
	//		ImGui::Text("%s", SDL_GetScancodeName(mapeamentos[i]));
	//	}
	//	if (aguardandoBotao != -1) {
	//		ImGui::Text(u8"Pressione uma tecla para o botão: %s", nomesBotoesNES[aguardandoBotao]);

	//		const Uint8* state = SDL_GetKeyboardState(nullptr);
	//		for (int sc = 0; sc < SDL_NUM_SCANCODES; ++sc) {
	//			if (state[sc]) {
	//				mapeamentos[aguardandoBotao] = static_cast<SDL_Scancode>(sc);
	//				aguardandoBotao = -1;
	//				break;
	//			}
	//		}
	//	}
	//	ImGui::End();
	//}

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
		// mantém o menu bar no topo quando não é fullscreen
		float menuBarAltura = ImGui::GetFrameHeight();
		offsetX = 0;
		offsetY = static_cast<int>(menuBarAltura);
	}

	SDL_Rect dstRect = { offsetX, offsetY, dstW, dstH };
	SDL_RenderCopy(RENDERER, TEXTURE, nullptr, &dstRect);


	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), RENDERER);

	SDL_RenderPresent(RENDERER);

	// INFELIZMENTE SO DA PRA USAR COM A BRANCH DOCCKING DO IMGUI, se alguem quiser mudar, sinta-se a vontade

	//// atualiza as janelas adicionais pra fora do main viewport
	//ImGuiIO& io = ImGui::GetIO();
	//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	//{
	//	ImGui::UpdatePlatformWindows();
	//	ImGui::RenderPlatformWindowsDefault();
	//}

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
		//self->apu->step(); // avança todos os canais
		//buffer[i] = self->apu->getMixedSample(); // retorna o som combinado dos canais
		const float cyclesPerSample = 1789773.0f / 44100.0f;
		self->apu->stepCpuCycles(cyclesPerSample);
		buffer[i] = self->apu->getMixedSample();
	}
}

void SDL_Display::inicializarAudio() {
	// Fecha o dispositivo de áudio anterior, se existir
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

void SDL_Display::setFirebaseAuth(firebase::auth::Auth* auth) {
	firebaseAuth = auth;
}

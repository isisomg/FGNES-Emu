#include "CPU.h"
#include "Bus.h"
#include "APU.h"
#include "SDL_Display.h"
#include "Cartucho.h"
#include <iostream>
#include <fstream>
#include "Controles.h"

#include "firebase/app.h"
#include "firebase/auth.h"
#include "firebase/log.h"

const int ciclosPorFrame = 29781;

int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		std::cerr << "Erro ao inicializar SDL: " << SDL_GetError() << std::endl;
		return 1;
	}

	std::cout << "Tentando inicializar Firebase..." << std::endl;

	firebase::AppOptions options;
	options.set_api_key("SUA_API_KEY");          // Encontre em Configurações do Projeto -> Geral
	options.set_app_id("SEU_APP_ID");            // Encontre em Configurações do Projeto -> Geral (ex: 1:1234567890:web:abcdef123456)
	options.set_project_id("SEU_PROJECT_ID");    // Encontre em Configurações do Projeto -> Geral

	firebase::App* firebase_app = firebase::App::Create(options);

	if (firebase_app) {
		std::cout << "Firebase App inicializado com sucesso!" << std::endl;
		std::cout << "Nome do App: " << firebase_app->name() << std::endl;
		std::cout << "App ID: " << firebase_app->options().app_id() << std::endl;

		// Opcional: Tentar obter o objeto Auth para testar a lib firebase_auth
		firebase::auth::Auth* auth = firebase::auth::Auth::GetAuth(firebase_app);
		if (auth) {
			std::cout << "Firebase Auth obtido com sucesso!" << std::endl;
		}
		else {
			std::cerr << "Erro ao obter Firebase Auth." << std::endl;
		}

	}
	else {
		std::cerr << "Erro ao inicializar Firebase App." << std::endl;
		// Você pode tentar obter mais detalhes do log se configurar firebase::log::SetLevel(firebase::log::kLogLevelVerbose);
		// antes de firebase::App::Create()
	}

	Bus* bus = new Bus();
	Cartucho cartucho;
	
	Controles controle;

	if (!controle.carregarMapeamento()) { //carrega mapeamento e cria arquivo padrão se necessario

		//Se carregarMapeamento retorna false salvamos o mapeamento padrão no json
		controle.salvarMapInicial();
	}

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
					apu.step();
					ppu.step(); ppu.step(); ppu.step();
				}
			}
			//std::cout << std::hex << (int)cpu.PC << std::endl;

		}
		
		display.renderizar();
		
	}

	//Teste salvar mapeamento, provavelmente vou remover no futuro
	std::cout << "Saindo... tentando salvar o mapeamento dos controles." << std::endl;
	controle.salvarMapeamento();

	if (firebase_app) {
		// Se você criou um AuthStateListener, remova-o aqui
		// if (auth_listener_ && auth) {
		//    auth->RemoveAuthStateListener(auth_listener_);
		//    delete auth_listener_;
		//    auth_listener_ = nullptr;
		// }
		// O objeto auth é gerenciado pelo firebase_app, não delete diretamente.
		delete firebase_app;
		firebase_app = nullptr;
		std::cout << "Firebase App finalizado." << std::endl;
	}

	//salvarArquivo(); // PARA DEBUG CPU
	display.destroy();
	delete bus;
	return 0;
}
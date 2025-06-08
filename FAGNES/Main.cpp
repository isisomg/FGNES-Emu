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
#include "firebase/database.h"
#include "utils.h"

const int ciclosPorFrame = 29781;

int main(int argc, char* argv[]) {
	SDL_Display display;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		std::cerr << "Erro ao inicializar SDL: " << SDL_GetError() << std::endl;
		return 1;
	}

	std::cout << "Tentando inicializar Firebase..." << std::endl;

	firebase::AppOptions options;
	options.set_api_key("AIzaSyCSCplMociGdPynKD4x0d_NA0wE77sZs8E");
	options.set_app_id("1:581274369648:web:b03a83d3abab3c3ffb3eb5");
	options.set_project_id("fagnes-db");

	firebase::App* firebase_app = firebase::App::Create(options);
	firebase::database::Database* db = firebase::database::Database::GetInstance(firebase_app, "https://fagnes-db-default-rtdb.firebaseio.com/");
	display.setFirebaseDatabase(db);

	if (firebase_app) {
		std::cout << "Firebase App inicializado com sucesso!" << std::endl;
		std::cout << "Nome do App: " << firebase_app->name() << std::endl;
		std::cout << "App ID: " << firebase_app->options().app_id() << std::endl;

		// Opcional: Tentar obter o objeto Auth para testar a lib firebase_auth
		firebase::auth::Auth* auth = firebase::auth::Auth::GetAuth(firebase_app);
		display.setFirebaseAuth(auth);
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

	std::string execDir = getExecutableDir();
	std::string caminhoP1 = execDir + "\\controles_p1.json";
	std::string caminhoP2 = execDir + "\\controles_p2.json";

	Controles controleP1;
	Controles controleP2;

	if (!controleP1.carregarMapeamento(caminhoP1)) { //carrega mapeamento e cria arquivo padrão se necessario

		//Se carregarMapeamento retorna false salvamos o mapeamento padrão no json
		controleP1.salvarMapInicial(caminhoP1);
	}
	if (!controleP2.carregarMapeamento(caminhoP2)) { //carrega mapeamento e cria arquivo padrão se necessario

		//Se carregarMapeamento retorna false salvamos o mapeamento padrão no json
		controleP2.salvarMapInicial(caminhoP2);
	}

	bus->setControlesP1(&controleP1);
	bus->setControlesP2(&controleP2);

	CPU cpu;
	PPU ppu;
	APU apu;
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

				if (cpu.ocorreuDMA) {
					cpu.ocorreuDMA = false;
					int ciclosStallDMA = 514;
					cpuCiclos += ciclosStallDMA;

					for (int c = 0; c < ciclosStallDMA; ++c) {
						apu.step();
						ppu.step(); ppu.step(); ppu.step();
					}
				}
			}
			//std::cout << std::hex << (int)cpu.PC << std::endl;

		}

		display.renderizar();

	}

	//Teste salvar mapeamento, provavelmente vou remover no futuro
	std::cout << "Saindo... tentando salvar o mapeamento dos controles." << std::endl;
	//controle.salvarMapeamento();
	std::string caminhoP1_final = getExecutableDir() + "\\controles_p1.json";
	std::string caminhoP2_final = getExecutableDir() + "\\controles_p2.json";
	controleP1.salvarMapeamento(caminhoP1_final);
	controleP2.salvarMapeamento(caminhoP2_final);

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
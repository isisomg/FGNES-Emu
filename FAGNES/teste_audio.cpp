//#include <iostream>
//#include <fstream>
//#include <SDL.h>
//#include "APU.h"
//#include "Bus.h"
//
//constexpr int sampleRate = 44100;
//constexpr float cpuCyclesPerSample = 1789773.0f / sampleRate;
//
//Bus dummyBus;
//APU apu;
//std::ofstream csv;
//
//bool running = true;
//
//// Callback chamado pelo SDL para preencher o buffer de áudio
//void audioCallback(void* userdata, Uint8* stream, int len) {
//    float* buffer = reinterpret_cast<float*>(stream);
//    int samples = len / sizeof(float);
//
//    for (int i = 0; i < samples; i++) {
//        apu.stepCpuCycles(cpuCyclesPerSample);
//
//        float pulse1 = apu.getSample(1);
//        float pulse2 = apu.getSample(2);
//        float triangle = apu.getSample(3);
//        float noise = apu.getSample(4);
//        float dmc = apu.getSample(5);
//        float mixed = apu.getMixedSample();
//
//        buffer[i] = mixed;
//
//        if (csv.is_open()) {
//            static int sampleIndex = 0;
//            csv << sampleIndex++ << "," << pulse1 << "," << pulse2 << "," << triangle << "," << noise << "," << dmc << "," << mixed << "\n";
//        }
//    }
//}
//
//int main(int argc, char* argv[]) {
//    // --- Inicialização da APU ---
//    apu.setBus(&dummyBus);
//    apu.setEnabled(true);
//
//    // Pulse 1
//    apu.writeRegister(0x4000, 0b00111111);
//    apu.writeRegister(0x4002, 0x00);
//    apu.writeRegister(0x4003, 0x07);
//
//    // Pulse 2
//    apu.writeRegister(0x4004, 0b01100001);
//    apu.writeRegister(0x4006, 0xFF);
//    apu.writeRegister(0x4007, 0x07);
//
//    // Triangle - Adicionar configuração
//    apu.writeRegister(0x4008, 0b11111111); // Linear counter control flag = 1 (ativa halt do length counter), reload value = 127
//    apu.writeRegister(0x400A, 0x88);       // Timer low (ex: período um pouco maior para tom mais baixo)
//    apu.writeRegister(0x400B, 0b00001011);
//
//    // Noise
//    apu.writeRegister(0x400C, 0b00111111); // LC Halt/Env Loop, Const Vol, Vol = 15
//    apu.writeRegister(0x400E, 0b00000011); // Mode 0, Period index 3 (era 0b000011, sem diferença)
//    apu.writeRegister(0x400F, 0x00);
//
//    // DMC
//    apu.writeRegister(0x4010, 0x4F); // IRQ off, Loop ON, Rate (ex: máximo, 0x0F, ou 0x00 como antes 0x40)
//    apu.writeRegister(0x4011, 0x40); // Nível de saída inicial 64
//    apu.writeRegister(0x4012, 0x00); // Sample address C000 + (0 * 64) = $C000
//    apu.writeRegister(0x4013, 0x0F);
//
//    // Habilita todos os canais
//    apu.writeRegister(0x4015, 0x1F);
//
//
//    // --- CSV ---
//    csv.open("audio_amostras.csv");
//    if (!csv.is_open()) {
//        std::cerr << "Erro ao abrir CSV\n";
//        return 1;
//    }
//    csv << "sample,pulse1,pulse2,triangle,noise,dmc,mixed\n";
//
//    // --- SDL ---
//    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
//        std::cerr << "Erro ao inicializar SDL: " << SDL_GetError() << "\n";
//        return 1;
//    }
//
//    SDL_AudioSpec desiredSpec{};
//    desiredSpec.freq = sampleRate;
//    desiredSpec.format = AUDIO_F32SYS;
//    desiredSpec.channels = 1;
//    desiredSpec.samples = 512;
//    desiredSpec.callback = audioCallback;
//
//    if (SDL_OpenAudio(&desiredSpec, nullptr) < 0) {
//        std::cerr << "Erro ao abrir áudio: " << SDL_GetError() << "\n";
//        SDL_Quit();
//        return 1;
//    }
//
//    // Cria uma janela simples
//    SDL_Window* window = SDL_CreateWindow("Teste de Áudio APU",
//        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
//        400, 200, SDL_WINDOW_SHOWN);
//
//    if (!window) {
//        std::cerr << "Erro ao criar janela: " << SDL_GetError() << "\n";
//        SDL_CloseAudio();
//        SDL_Quit();
//        return 1;
//    }
//
//    SDL_PauseAudio(0); // Inicia o áudio
//
//    std::cout << "Pressione Q para sair...\n";
//
//    // --- Loop principal SDL ---
//    SDL_Event e;
//    while (running) {
//        while (SDL_PollEvent(&e)) {
//            if (e.type == SDL_QUIT) {
//                running = false;
//            }
//            else if (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_q || e.key.keysym.sym == SDLK_ESCAPE)) {
//                running = false;
//            }
//        }
//
//        SDL_Delay(16); // ~60 FPS
//    }
//
//    SDL_CloseAudio();
//    SDL_DestroyWindow(window);
//    SDL_Quit();
//    csv.close();
//
//    std::cout << "Encerrado com sucesso.\n";
//    return 0;
//}

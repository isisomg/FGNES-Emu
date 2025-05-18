//#define SDL_MAIN_HANDLED
//#include <SDL.h>
//#include <iostream>
////#include "audio.h"
//#include <fstream>
//#include <algorithm>
//#include "APU.h"
//
//APU pulse1;
//APU pulse2;
//APU triangle;
//APU noise;
//APU dmc;
//
//void setupPulses() {
//	float canais = 8.0f; // 8 canais do duty
//
//    pulse1.setDuty(2);        // 50% duty
//    pulse1.setFrequency(440, canais);
//    pulse1.setVolume(32);     // Volume razo�vel
//    pulse2.setDuty(1);        // 25%
//    pulse2.setFrequency(660, canais);
//    pulse2.setVolume(8);      // Mais fraco
//}
//
//void setupTriangle() {
//	float canais = 32.0f; // 32 canais da tabela
//
//    triangle.setEnabled(true);
//    triangle.setFrequency(220, canais);
//}
//
//void setupNoise() {
//	float canal = 1.0f; // 1 canal do noise
//
//    noise.setEnabled(true);
//    noise.setVolume(6);
//    noise.setMode(false);      // false = modo longo, true = curto
//    noise.setFrequency(4000, canal);
//}
//
//void setupDMC() {
//    dmc.setEnabled(true); // Apenas ativa o canal
//}
//
//
////void setupDMC() {
////    dmc.setSampleData(APU::fakeSample, sizeof(APU::fakeSample));
////    dmc.setSampleAddress(0);       // �ndice inicial do sample
////    dmc.setSampleLength(sizeof(APU::fakeSample));
////    dmc.setRate(0);        // index 0 = 428 ciclos
////    dmc.setLoop(true);
////    dmc.dmcSetEnabled(true);
////}
//
////DMCChannel dmc;
//// 
////void setupDMC() {
////    dmc.setSampleData(DMCChannel::fakeSample, sizeof(DMCChannel::fakeSample));
////    dmc.setSampleAddress(0);       // �ndice inicial do sample
////    dmc.setSampleLength(sizeof(DMCChannel::fakeSample));
////    dmc.setRate(0);        // index 0 = 428 ciclos
////    dmc.setLoop(true);
////    dmc.setEnabled(true);
////}
//
////std::ofstream triangleOut("triangle_samples.csv"); // isso � para gerar um .csv para visualizar se o formato das ondas est� certo
////std::ofstream pulseOut("pulse_samples.csv");
//
////void audioCallback(void* userdata, Uint8* stream, int len) {
////    float* buffer = (float*)stream;
////    int samples = len / sizeof(float);
////    for (int i = 0; i < samples; ++i) {
////        pulse1.tick();
////        pulse2.tick();
////        triangle.tick();
////        noise.tick();
////
////        float samplePulse = pulse1.getSample() + pulse2.getSample();
////        float sampleTriangle = triangle.getSample();
////        float mixed = (samplePulse + sampleTriangle) / 2.0f;
////
////        // Grava os samples de pulse em CSV
////        if (i % 2 == 0 && pulseOut.is_open()) {
////            pulseOut << samplePulse << "\n";
////        }
////
////        // Grava triangle como antes
////        if (i % 2 == 0 && triangleOut.is_open()) {
////            triangleOut << sampleTriangle << "\n";
////        }
////
////        buffer[i] = mixed;
////    }
////}
//
//void audioCallback(void* userdata, Uint8* stream, int len) {
//    float* buffer = (float*)stream;
//    int samples = len / sizeof(float);
//    for (int i = 0; i < samples; ++i) {
//        pulse1.tick(1);
//        pulse2.tick(1);
//        triangle.tick(2);
//        noise.tick(3);
//        dmc.tick(4);
//
//        float sample = pulse1.getSample(1) + pulse2.getSample(1) + triangle.getSample(2) + noise.getSample(3) + dmc.getSample(4);
//
//        // Clipping prevention simples
//        if (sample > 1.0f) sample = 1.0f;
//        buffer[i] = sample;
//    }
//}
//
//int main() {
//    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
//        std::cerr << "Erro SDL: " << SDL_GetError() << "\n";
//        return 1;
//    }
//
//    /*AudioSystem audio;*/
//    //audio.pulse.setVolume(15);     // volume m�ximo
//    //audio.pulse.setDuty(2);        // 50%
//    //audio.pulse.setTimer(40);     // frequ�ncia ~ NES 441000 / 200 * 8, quanto menor, mais agudo, quanto maior, mais grave
//    //audio.pulse.resetPhase();
//    //audio.start();
//
//    SDL_AudioSpec want{};
//    want.freq = 44100; // frequencia em hertz do dispositivo de audio
//    want.format = AUDIO_F32;
//    want.channels = 1; // canal mono, o NES funcionava apenas com um canal de audio
//    want.samples = 512; // tamanho do buffer de audio, pede n amostras de audio por vez
//    want.callback = audioCallback;
//
//
//    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);
//    if (!device) {
//        SDL_Log("Erro ao abrir o dispositivo de �udio: %s", SDL_GetError());
//        SDL_Quit();
//        return 1;
//    }
//
//    setupPulses();
//    setupTriangle();
//    setupNoise();
//	setupDMC();
//
//    SDL_PauseAudioDevice(device, 0); // inicia audio
//
//    // OBS cria uma janela so pra eu poder testar mesmo, pode tirar isso depois
//
//    std::cout << "Tocando som... Pressione Q ou feche a janela para sair.\n";
//    SDL_Window* window = SDL_CreateWindow("Emulador APU NES",
//        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
//        640, 480, SDL_WINDOW_SHOWN);
//
//    if (!window) {
//        std::cerr << "Erro ao criar janela: " << SDL_GetError() << "\n";
//        SDL_Quit();
//        return 1;
//    }
//
//    SDL_Event e;
//    bool running = true;
//    while (running) {
//        while (SDL_PollEvent(&e)) {
//            if (e.type == SDL_QUIT) {
//                running = false;
//            }
//            else if (e.type == SDL_KEYDOWN) {
//                if (e.key.keysym.sym == SDLK_q) {
//                    running = false;
//                }
//            }
//        }
//        SDL_Delay(16); // Aproximadamente 60 FPS
//    }
//
//
//    /*std::cin.get();*/
//    SDL_CloseAudioDevice(device);
//    SDL_DestroyWindow(window);
//
//    /*audio.stop();*/
//
//    SDL_Quit();
//    return 0;
//}
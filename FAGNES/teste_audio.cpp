//#define SDL_MAIN_HANDLED
//#include <SDL.h>
//#include <iostream>
////#include "audio.h"
//#include <fstream>
//#include <algorithm>
//#include "pulse_channel.h"
//#include "TriangleChannel.h"
//#include "NoiseChannel.h"
//#include "DMCChannel.h"
//
//DMCChannel dmc;
//
//void setupDMC() {
//    dmc.setSampleData(DMCChannel::fakeSample, sizeof(DMCChannel::fakeSample));
//    dmc.setSampleAddress(0);       // índice inicial do sample
//    dmc.setSampleLength(sizeof(DMCChannel::fakeSample));
//    dmc.setRate(0);        // index 0 = 428 ciclos (o mesmo valor que você queria)
//    dmc.setLoop(true);     // usa o nome correto do método
//    dmc.setEnabled(true);
//}
//
//PulseChannel pulse1;
//PulseChannel pulse2;
//TriangleChannel triangle;
//NoiseChannel noise;
//
////std::ofstream triangleOut("triangle_samples.csv"); // isso é para gerar um .csv para visualizar se o formato das ondas está certo
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
//        /*pulse1.tick();
//        pulse2.tick();
//        triangle.tick();
//        noise.tick();*/
//        dmc.tick();
//
//        float sample = /*pulse1.getSample() + pulse2.getSample() + triangle.getSample() + noise.getSample() + */dmc.getSample();
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
//    //audio.pulse.setVolume(15);     // volume máximo
//    //audio.pulse.setDuty(2);        // 50%
//    //audio.pulse.setTimer(40);     // frequência ~ NES 441000 / 200 * 8, quanto menor, mais agudo, quanto maior, mais grave
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
//        SDL_Log("Erro ao abrir o dispositivo de áudio: %s", SDL_GetError());
//        SDL_Quit();
//        return 1;
//    }
//
//    pulse1.setDuty(2);        // 50% duty
//    pulse1.setFrequency(440); 
//    pulse1.setVolume(12);     // Volume razoável
//
//    pulse2.setDuty(1);        // 25%
//    pulse2.setFrequency(660); 
//    pulse2.setVolume(8);      // Mais fraco
//
//    triangle.setEnabled(true);
//    triangle.setFrequency(220); 
//
//    noise.setEnabled(true);
//    noise.setVolume(6);
//    noise.setMode(false);      // false = modo longo, true = curto
//    noise.setFrequency(4000);   // ou experimente com 100, 500, 1000 Hz
//
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
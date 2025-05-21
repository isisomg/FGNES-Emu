#pragma once
#include <SDL.h>
#include <iostream>
#include "Bus.h"
#include "Cores.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include "Cartucho.h"
#include "CPU.h"

class SDL_Display {
private:
    APU* apu = nullptr;
    PPU* ppu = nullptr;
    SDL_AudioDeviceID audioDevice;

    static void audioCallback(void* userdata, Uint8* stream, int len);

    void inicializarAudio();
    const int TELA_WIDTH = 256;
    const int TELA_HEIGHT = 240;
    int ZOOM = 1;
    int prevZOOM = 1;
    bool isFull = false;
    const int FPS = 30;

    SDL_Texture* TEXTURE;
    SDL_Window* WINDOW;

    Bus* bus;
    Cartucho* cartucho;

public:
    bool jogoRodando = false;

    SDL_Renderer* RENDERER;

    //SDL_Display();
    void init(Bus* novoBus, Cartucho* cartucho, PPU* p);
    void processarEntrada(SDL_Event event);
    void renderizar();
    void destroy();

    //~SDL_Display();
};

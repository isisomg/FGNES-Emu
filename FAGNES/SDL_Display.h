#pragma once
#include <SDL.h>
#include <iostream>
#include "Memoria.h"
#include "Cores.h"

class SDL_Display {
private:
    const int TELA_WIDTH = 32;
    const int TELA_HEIGHT = 32;
    int ZOOM = 8;
    const int FPS = 30;

    SDL_Renderer* RENDERER;
    SDL_Texture* TEXTURE;
    SDL_Window* WINDOW;

public:
    //SDL_Display();

    void init();
    void processarEntrada(SDL_Event event, Memoria& mem);
    void renderizar(Memoria& mem);
    void destroy();

    //~SDL_Display();
};

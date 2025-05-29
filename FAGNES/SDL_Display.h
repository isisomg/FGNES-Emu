#pragma once
#include <SDL.h>
#include <iostream>
#include <string>
#include "Bus.h"
#include "Cores.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include "Cartucho.h"
#include "CPU.h"
#include <map>
#include "firebase/auth.h"

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

    // Nova janela de controle
    SDL_Window* controleWindow = nullptr;
    SDL_Renderer* controleRenderer = nullptr;
    bool janelaControleAberta = false;

    std::map<std::string, SDL_Scancode> mapeamentoTeclas;
    bool mostrarJanelaControle = false;
    bool mostrarJanelaConta = false;
    bool usuarioLogado = false;
    std::string botaoSelecionado = "";
    std::string nomeUsuarioLogado = "";
    int botaoAguardandoMapeamento = -1;
    bool mostrarPopupTeclaEmUso = false;
    std::string mensagemPopupTeclaEmUso = "";
    bool exibirMensagemErroLogin = false;
    std::string mensagemErroLogin = "";
    bool exibirErroCadastro = false;
    std::string erroCadastro = "";

    enum class EstadoConta {
        FazendoLogin,
        LoginSucesso,
        Logado,
        Cadastro
    };
    EstadoConta estadoAtualConta = EstadoConta::FazendoLogin;
    firebase::auth::Auth* firebaseAuth = nullptr;

public:
    bool jogoRodando = false;

    SDL_Renderer* RENDERER;

    //SDL_Display();
    void init(Bus* novoBus, Cartucho* cartucho, PPU* p);
    void processarEntrada(SDL_Event event);
    void renderizar();
    void destroy();
    void setFirebaseAuth(firebase::auth::Auth* auth);
    static void CallbackLogin(const firebase::Future<firebase::auth::AuthResult>* future, void* user_data);
    static void CallbackCadastro(const firebase::Future<firebase::auth::AuthResult>* future, void* user_data);

    // M�todos novos para a janela de controle
    //void abrirJanelaControle();
    //void renderizarJanelaControle();
};
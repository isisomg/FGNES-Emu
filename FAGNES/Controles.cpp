#include "Controles.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

//Implementação das funções auxiliares
const char* botaoParaString(botoesNES botao) {
    switch (botao) {
    case botoesNES::A: return "A";
    case botoesNES::B: return "B";
    case botoesNES::START: return "START";
    case botoesNES::SELECT: return "SELECT";
    case botoesNES::UP: return "UP";
    case botoesNES::DOWN: return "DOWN";
    case botoesNES::LEFT: return "LEFT";
    case botoesNES::RIGHT: return "RIGHT";
    default: return "UNKNOWN";
    }
}

botoesNES botaoParaString(const std::string& s) {
    if (s == "A") return botoesNES::A;
    if (s == "B") return botoesNES::B;
    if (s == "START") return botoesNES::START;
    if (s == "SELECT") return botoesNES::SELECT;
    if (s == "UP") return botoesNES::UP;
    if (s == "DOWN") return botoesNES::DOWN;
    if (s == "LEFT") return botoesNES::LEFT;
    if (s == "RIGHT") return botoesNES::RIGHT;
    return botoesNES::A; //default em caso de erro
}

Controles::Controles() {
    //Cria o mapeamento padrão para os controles, caso não exista arquivo .json carregado
    mapeamentoPadrao[botoesNES::A] = SDL_SCANCODE_Z;
    mapeamentoPadrao[botoesNES::B] = SDL_SCANCODE_X;
    mapeamentoPadrao[botoesNES::START] = SDL_SCANCODE_D;
    mapeamentoPadrao[botoesNES::SELECT] = SDL_SCANCODE_F;
    mapeamentoPadrao[botoesNES::UP] = SDL_SCANCODE_UP;
    mapeamentoPadrao[botoesNES::DOWN] = SDL_SCANCODE_DOWN;
    mapeamentoPadrao[botoesNES::LEFT] = SDL_SCANCODE_LEFT;
    mapeamentoPadrao[botoesNES::RIGHT] = SDL_SCANCODE_RIGHT;

    mapeamentoTeclas = mapeamentoPadrao;
}

void Controles::pressionar(botoesNES botao) {
	estado |= (1 << static_cast<int>(botao));
    //printf("Controle: botão %d pressionado, estado = 0x%02X\n", botao, estado);
}

void Controles::soltar(botoesNES botao) {
	estado &= ~(1 << static_cast<int>(botao));
}

void Controles::escreverStrobe(bool valor) {
    strobe = valor;
    if (strobe) {
        indice = 0;
    }
}

Byte Controles::ler() {
    Byte retorno = 0;

    if (indice < static_cast<int>(botoesNES::COUNT)) {
        retorno = (estado >> indice) & 1;
    }
    else {
        retorno = 1; // Após 8 leituras, o NES retorna 1
    }

    if (!strobe) {
        indice++;
    }

    //printf("Controle: indice=%d, bit=%d\n", indice - 1, retorno);
    return retorno;
}

void Controles::processarEntrada(SDL_Scancode scancode, bool pressionado) {
    for (auto const& [botaoEnum, scancodeMapeado] : mapeamentoTeclas) {
        if (scancode == scancodeMapeado) {
            if (pressionado) {
                pressionar(botaoEnum);
            }
            else {
                soltar(botaoEnum);
            }
            // std::cout << "Botao NES " << botaoParaString(botaoEnum) << (pressionado ? " pressionado" : " solto") << std::endl;
            break;
        }
    }
}

bool Controles::carregarMapeamento(const std::string& caminhoArquivo) {
    std::ifstream arquivo(caminhoArquivo);
    if (!arquivo.is_open()) {
        std::cerr << "Aviso: Não foi possivel abrir o arquivo de mapeamento: " << caminhoArquivo << ". Usando mapeamento padrão." << std::endl;
        mapeamentoTeclas = mapeamentoPadrao; //usa o mapeamento padrão
        return false; //retorna que o carregamento do arquivo falhou
    }

    try {
        json j;
        arquivo >> j;
        std::map<botoesNES, SDL_Scancode> novoMapeamento = mapeamentoPadrao; //começa com o mapeamento padrão para preencher os faltantes

        for (int i = 0; i < static_cast<int>(botoesNES::COUNT); i++) {
            botoesNES botao = static_cast<botoesNES>(i);
            const char* nomeBotaoStr = botaoParaString(botao);

            if (j.contains(nomeBotaoStr)) {
                std::string nomeTeclaJson = j[nomeBotaoStr].get<std::string>();
                SDL_Scancode sc = SDL_GetScancodeFromName(nomeTeclaJson.c_str());
                if (sc != SDL_SCANCODE_UNKNOWN) {
                    novoMapeamento[botao] = sc;
                }
                else {
                    std::cerr << "Aviso: Nome de tecla desconhecido '" << nomeTeclaJson << "' para o botao " << nomeBotaoStr << " no JSON. Usando padrao para este botao." << std::endl;
                    // novoMapeamento[botao] já tem o padrão
                }
            }
            else {
                // std::cerr << "Aviso: Botao " << nomeBotaoStr << " nao encontrado no JSON. Usando padrao para este botao." << std::endl;
                 // novoMapeamento[botao] já tem o padrão
            }
        }
        mapeamentoTeclas = novoMapeamento;
        std::cout << "Mapeamento de controles carregado de " << caminhoArquivo << std::endl;
        return true;

    }
    catch (json::exception& e) {
        std::cerr << "Erro ao fazer parse do JSON: " << e.what() << ". Usando mapeamento padrao." << std::endl;
        mapeamentoTeclas = mapeamentoPadrao;
        return false;
        }   

}

bool Controles::salvarMapInicial(const std::string& caminhoArquivo) {

    std::ifstream verificaArquivo(caminhoArquivo);
    if (verificaArquivo.good()) {
        verificaArquivo.close();
        return true;
    }
    verificaArquivo.close();

    json j;
    for (auto const& [botao, scancode] : mapeamentoTeclas) {
        const char* nomeTecla = SDL_GetScancodeName(scancode);
        
        if (nomeTecla && nomeTecla[0] != '\0') {
            j[botaoParaString(botao)] = nomeTecla;
        }
        else {
            j[botaoParaString(botao)] = "UNKNOWN";
        }
    }

    std::ofstream arquivo(caminhoArquivo);

    if (!arquivo.is_open()) {
        std::cerr << "Erro: Nao foi possivel criar o arquivo de mapeamento inicial: " << caminhoArquivo << std::endl;
        return false;

    } 
    try {

        arquivo << j.dump(4);
        std::cout << "Mapeamento padrao salvo em " << caminhoArquivo << std::endl;
        return true;

    }
    catch(json::exception& e) {
        std::cerr << "Erro ao serializar para JSON (inicial): " << e.what() << std::endl;
        return false;
    }

}
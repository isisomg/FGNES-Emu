#include "Controles.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"
#include <vector>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "firebase/database.h"
#include "firebase/variant.h"

using json = nlohmann::json;

bool Controles::salvarMapeamentoNoFirebase(firebase::database::Database* db, const std::string& user_id) {
    json j;
    for (auto const& [botaoEnum, scancode] : mapeamentoTeclas) {
        const char* nomeTecla = SDL_GetScancodeName(scancode);
        j[botaoParaString(botaoEnum)] = nomeTecla && nomeTecla[0] != '\0' ? nomeTecla : "UNKNOWN";
    }

    std::string jsonString = j.dump();
    auto ref = db->GetReference().Child("mapeamentos").Child(user_id);
    ref.SetValue(jsonString);
    return true;
}

bool Controles::carregarMapeamentoDoFirebase(firebase::database::Database* db, const std::string& user_id) {
    auto ref = db->GetReference().Child("mapeamentos").Child(user_id);
    firebase::Future<firebase::database::DataSnapshot> future = ref.GetValue();

    while (future.status() != firebase::kFutureStatusComplete) {
        SDL_Delay(10);
    }

    if (future.error() != 0) {
        std::cerr << "Erro ao carregar mapeamento do Firebase: " << future.error_message() << std::endl;
        return false;
    }

    std::string jsonStr = future.result()->value().string_value();
    json j = json::parse(jsonStr);

    for (int i = 0; i < static_cast<int>(botoesNES::COUNT); i++) {
        botoesNES botao = static_cast<botoesNES>(i);
        std::string nome = botaoParaString(botao);
        if (j.contains(nome)) {
            SDL_Scancode sc = SDL_GetScancodeFromName(j[nome].get<std::string>().c_str());
            if (sc != SDL_SCANCODE_UNKNOWN) {
                mapeamentoTeclas[botao] = sc;
            }
        }
    }

    return true;
}

//Função auxiliar para caminho do executável

//std::wstring getExecutableDirW() {
//    std::vector<wchar_t> buffer(MAX_PATH + 1);
//    DWORD bytes = GetModuleFileNameW(NULL, buffer.data(), static_cast<DWORD>(buffer.size()));
//    if (bytes == 0 || bytes >= buffer.size()) {
//        return L"";
//    }
//
//    std::wstring exePath(buffer.data());
//    size_t lastBackslash = exePath.find_last_of(L"\\/");
//    if (std::wstring::npos != lastBackslash) {
//        return exePath.substr(0, lastBackslash);
//    }
//    return L"";
//}

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

botoesNES stringParaBotao(const std::string& s) {
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
    //std::cout << "Botao PRESSIONADO: " << botaoParaString(botao) << ", Novo Estado: 0x" << std::hex << static_cast<int>(estado) << std::dec << std::endl;
}

void Controles::soltar(botoesNES botao) {
	estado &= ~(1 << static_cast<int>(botao));
    //std::cout << "Botao PRESSIONADO: " << botaoParaString(botao) << ", Novo Estado: 0x" << std::hex << static_cast<int>(estado) << std::dec << std::endl;
}

void Controles::escreverStrobe(bool valor) {
    strobe = valor;
    //std::cout << "STROBE escrito: " << (strobe ? "1 (LATCH/RESET INDICE)" : "0 (ENABLE SHIFT)") << std::endl;
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

    //std::cout << "Controle::ler() - Strobe: " << strobe << ", Indice Atual: " << static_cast<int>(indice) << ", Estado: 0x" << std::hex << static_cast<int>(estado) << std::dec << ", Retornando bit: " << static_cast<int>(retorno) << std::endl;

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

    //std::wstring execDir = getExecutableDirW();
    std::ifstream arquivo(caminhoArquivo);
    if (!arquivo.is_open()) {
        std::cerr << "ERRO: Nao foi possivel obter o diretorio do executavel. Usando mapeamento padrao." << std::endl;
        this->mapeamentoTeclas = this->mapeamentoPadrao;
        return false;
    }
    //std::wstring caminhoCompletoW = execDir + L"\\controles.json";

    //std::ifstream arquivo(caminhoCompletoW);
    //if (!arquivo.is_open()) {
    //    std::wcerr << "Aviso: Não foi possivel abrir o arquivo de mapeamento: " << caminhoCompletoW << ". Usando mapeamento padrão." << std::endl;
    //    mapeamentoTeclas = mapeamentoPadrao; //usa o mapeamento padrão
    //    return false; //retorna que o carregamento do arquivo falhou
    //}

    try {
        json j;
        arquivo >> j;
        std::map<botoesNES, SDL_Scancode> novoMapeamento = this->mapeamentoPadrao; //começa com o mapeamento padrão para preencher os faltantes

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
        this->mapeamentoTeclas = novoMapeamento;
        std::cerr << "Mapeamento de controles carregado de " << caminhoArquivo << std::endl;
        return true;

    }
    catch (json::exception& e) {
        std::cerr << "Erro ao fazer parse do JSON de: " << caminhoArquivo << L". " << e.what() << ". Usando mapeamento padrao." << std::endl;
        this->mapeamentoTeclas = this->mapeamentoPadrao;
        return false;
        }   

}

bool Controles::salvarMapeamento(const std::string& caminhoArquivo) {

    /*std::wstring execDir = getExecutableDirW();
    if (execDir.empty()) {
        std::cerr << "ERRO: Nao foi possivel obter o diretorio do executavel. Mapeamento NAO salvo." << std::endl;
        return false;
    }
    std::wstring caminhoCompletoW = execDir + L"\\controles.json";*/


    json j;
    for (auto const& [botaoEnum, scancode] : this->mapeamentoTeclas) { //itera sobre o mapeamento de teclas atual e salva
        const char* nomeTecla = SDL_GetScancodeName(scancode);
        
        if (nomeTecla && nomeTecla[0] != '\0') {
            j[botaoParaString(botaoEnum)] = nomeTecla;
        }
        else {
            j[botaoParaString(botaoEnum)] = "UNKNOWN";
            std::cerr << "Aviso: Scancode para o botao " << botaoParaString(botaoEnum) << " nao tem nome ou é UNKNOWN. Salvo como UNKNOWN." << std::endl;
        }

    }

    std::ofstream arquivo(caminhoArquivo);

    if (!arquivo.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo para salvar o mapeamento: " << caminhoArquivo << std::endl;
        return false;
    }

    try {
        arquivo << j.dump(4);
        std::cout << "Mapeamento de controles salvo em " << caminhoArquivo << std::endl;
        return true;
    }
    catch(json::exception& e){
        std::cerr << "Erro ao serializar para JSON ao salvar: " << e.what() << std::endl;
        return false;
    }
}

bool Controles::salvarMapInicial(const std::string& caminhoArquivo) {

    //std::wstring execDir = getExecutableDirW();
    //if (execDir.empty()) {
    //    std::cerr << "ERRO: Nao foi possivel obter o diretorio do executavel para salvar o mapa inicial." << std::endl;
    //    return false; // Não podemos verificar ou salvar se não sabemos onde.
    //}
    //std::wstring caminhoCompletoW = execDir + L"\\controles.json";


    std::ifstream verificaArquivo(caminhoArquivo);
    if (verificaArquivo.good()) {
        verificaArquivo.close();
        return true;
    }
    verificaArquivo.close();

    std::cout << L"Arquivo de mapeamento inicial nao encontrado em " << caminhoArquivo << L". Criando um novo com os padroes." << std::endl;

    return salvarMapeamento(caminhoArquivo);

    /*json j;
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
    }*/

}

SDL_Scancode Controles::getScancodeParaBotao(botoesNES botao) const {

    auto it = mapeamentoTeclas.find(botao);
    if (it != mapeamentoTeclas.end()) {
        return it->second;
    }

    auto it_padrao = mapeamentoPadrao.find(botao);
    if (it_padrao != mapeamentoPadrao.end()) {
        return it_padrao->second;
    }

    return SDL_SCANCODE_UNKNOWN;
}

bool Controles::setScancodeParaBotao(botoesNES botaoAlvo, SDL_Scancode novoScancode, botoesNES* pBotaoConflito) {

    if (pBotaoConflito) {

    }

    if (novoScancode == SDL_SCANCODE_UNKNOWN) {
        mapeamentoTeclas[botaoAlvo] = SDL_SCANCODE_UNKNOWN;
        std::cout << "Botao " << botaoParaString(botaoAlvo) << " foi desmapeado." << std::endl;
        return true;
    }

    for (auto const& [botaoExistente, scancodeExistente] : mapeamentoTeclas) {
        if (scancodeExistente == novoScancode && botaoExistente != botaoAlvo) {
            std::cerr << "ERRO: Tecla " << SDL_GetScancodeName(novoScancode) << " ja esta mapeada para o botao " << botaoParaString(botaoExistente) << std::endl;
            if (pBotaoConflito) {
                *pBotaoConflito = botaoExistente;
            }
            return false;
        }
    }

    mapeamentoTeclas[botaoAlvo] = novoScancode;
    std::cout << "Botao " << botaoParaString(botaoAlvo) << " mapeado para a tecla '" << SDL_GetScancodeName(novoScancode) << "'." << std::endl;
    return true;

    /*if (scancode != SDL_SCANCODE_UNKNOWN) {
        mapeamentoTeclas[botao] = scancode;
        std::cout << "Botao " << botaoParaString(botao) << " remapeado para " << SDL_GetScancodeName(scancode) << std::endl;

    }
    else {
        std::cout << "Tentativa de mapear Botao " << botaoParaString(botao) << " para SDL_SCANCODE_UNKNOWN ignorada." << std::endl;
    }*/
}

void Controles::reverterParaPadrao() {

    mapeamentoTeclas = mapeamentoPadrao;
    std::cout << "Mapeamento de controles revertido para o padrao." << std::endl;

}
#pragma once
#include "Tipos.h"
#include <string>
#include <map>
#include <SDL_scancode.h>
#include <SDL_keyboard.h>

//índices dos botões
enum class botoesNES {

	A = 0,
	B = 1,
	SELECT = 2,
	START = 3,
	UP = 4,
	DOWN = 5,
	LEFT = 6,
	RIGHT = 7,
	COUNT

};

//Funções Auxiliares

const char* botaoParaString(botoesNES botao);
botoesNES stringParaBotao(const std::string& s);

//Classe controles
class Controles {

private:
	Byte estado = 0;
	Byte indice = 0;
	bool strobe = false;

	std::map<botoesNES, SDL_Scancode> mapeamentoTeclas; //mapa para armazenar qual scancode corresponde a qual botão
	std::map<botoesNES, SDL_Scancode> mapeamentoPadrao; //mapeamento padrão
	
public:
	Controles();

	void pressionar(botoesNES botao);
	void soltar(botoesNES botao);
	void escreverStrobe(bool valor);
	Byte ler();

	bool carregarMapeamento(const std::string& caminhoArquivo);
	void processarEntrada(SDL_Scancode scancode, bool pressionado);
	bool salvarMapInicial(const std::string& caminhoArquivo);
	bool salvarMapeamento(const std::string& caminhoArquivo);
};
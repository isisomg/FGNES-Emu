#include "Controles.h"
#include <stdio.h>

void Controles::pressionar(botoesNES botao) {
	estado |= (1 << botao);
    printf("Controle: botão %d pressionado, estado = 0x%02X\n", botao, estado);
}

void Controles::soltar(botoesNES botao) {
	estado &= ~(1 << botao);
}

void Controles::escreverStrobe(bool valor) {
    strobe = valor;
    if (strobe) {
        indice = 0;
    }
}

uint8_t Controles::ler() {
    uint8_t retorno = 0;

    if (indice < 8) {
        retorno = (estado >> indice) & 1;
    }
    else {
        retorno = 1; // Após 8 leituras, o NES retorna 1
    }

    if (!strobe) {
        indice++;
    }

    printf("Controle: indice=%d, bit=%d\n", indice - 1, retorno);
    return retorno;
}
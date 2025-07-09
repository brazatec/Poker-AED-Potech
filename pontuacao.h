#ifndef pontuacao_h
#define pontuacao_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>

// Modularização
#include "disco.h"
#include "baralho.h"

#define CARTAS 52
#define CARTAS_POR_JOGADOR 2
#define MAX_JOGADORES 5
#define MIN_JOGADORES 2
#define SMALL_BLIND 10
#define BIG_BLIND 20
#define MAX 52

int contarIguais(valor_carta *cartas, int total, int valor) {
    int cont = 0;
    for (int i = 0; i < total; i++) {
        if (cartas[i].valor == valor) cont++;
    }
    return cont;
}

int cartaAlta(valor_carta *cartas, int total) {
    int maior = 0;
    for (int i = 0; i < total; i++) {
        if (cartas[i].valor > maior) maior = cartas[i].valor;
    }
    return maior;
}

int temPar(valor_carta *cartas, int total) {
    for (int i = 0; i < total; i++) {
        if (contarIguais(cartas, total, cartas[i].valor) == 2)
            return 1;
    }
    return 0;
}

int temDoisPares(valor_carta *cartas, int total) {
    int pares = 0;
    int verificados[15] = {0};
    for (int i = 0; i < total; i++) {
        if (!verificados[cartas[i].valor] && contarIguais(cartas, total, cartas[i].valor) == 2) {
            pares++;
            verificados[cartas[i].valor] = 1;
        }
    }
    return pares >= 2;
}

int temTrinca(valor_carta *cartas, int total) {
    for (int i = 0; i < total; i++) {
        if (contarIguais(cartas, total, cartas[i].valor) == 3)
            return 1;
    }
    return 0;
}



int temStraight(valor_carta *cartas, int total) {
    int valores[15] = {0};
    for (int i = 0; i < total; i++) {
        valores[cartas[i].valor] = 1;
    }
    int sequencia = 0;
    for (int i = 2; i <= 14; i++) {
        if (valores[i]) {
            sequencia++;
            if (sequencia >= 5) return 1;
        } else {
            sequencia = 0;
        }
    }
    if (valores[14] && valores[2] && valores[3] && valores[4] && valores[5])
        return 1;
    return 0;
}

int temFlush(valor_carta *cartas, int total) {
    int cont[4] = {0};
    for (int i = 0; i < total; i++) {
        if (cartas[i].naipe == L'♣') cont[0]++;
        else if (cartas[i].naipe == L'♦') cont[1]++;
        else if (cartas[i].naipe == L'♥') cont[2]++;
        else if (cartas[i].naipe == L'♠') cont[3]++;
    }
    for (int i = 0; i < 4; i++) {
        if (cont[i] >= 5) return 1;
    }
    return 0;
}

int temFullHouse(valor_carta *cartas, int total) {
    int tem_trinca = 0, tem_par = 0;
    for (int i = 0; i < total; i++) {
        int count = contarIguais(cartas, total, cartas[i].valor);
        if (count == 3) tem_trinca = 1;
        else if (count == 2) tem_par = 1;
    }
    return tem_trinca && tem_par;
}

int temQuadra(valor_carta *cartas, int total) {
    for (int i = 0; i < total; i++) {
        if (contarIguais(cartas, total, cartas[i].valor) == 4)
            return 1;
    }
    return 0;
}

int temStraightFlush(valor_carta *cartas, int total) {
    int naipes[4][15] = {{0}};
    for (int i = 0; i < total; i++) {
        int naipe_idx = (cartas[i].naipe == L'♣') ? 0 : (cartas[i].naipe == L'♦') ? 1 : (cartas[i].naipe == L'♥') ? 2 : 3;
        naipes[naipe_idx][cartas[i].valor] = 1;
    }
    for (int n = 0; n < 4; n++) {
        int sequencia = 0;
        for (int i = 2; i <= 14; i++) {
            if (naipes[n][i]) {
                sequencia++;
                if (sequencia >= 5) return 1;
            } else {
                sequencia = 0;
            }
        }
        if (naipes[n][14] && naipes[n][2] && naipes[n][3] && naipes[n][4] && naipes[n][5])
            return 1;
    }
    return 0;
}

int temRoyalFlush(valor_carta *cartas, int total) {
    int naipes[4][15] = {{0}};
    for (int i = 0; i < total; i++) {
        int naipe_idx = (cartas[i].naipe == L'♣') ? 0 : (cartas[i].naipe == L'♦') ? 1 : (cartas[i].naipe == L'♥') ? 2 : 3;
        naipes[naipe_idx][cartas[i].valor] = 1;
    }
    for (int n = 0; n < 4; n++) {
        if (naipes[n][10] && naipes[n][11] && naipes[n][12] && naipes[n][13] && naipes[n][14])
            return 1;
    }
    return 0;
}

int pontuacaoMaoCompleta(valor_carta *cartas, int total, int *high_card) {
    if (temRoyalFlush(cartas, total)) return 9;
    if (temStraightFlush(cartas, total)) return 8;
    if (temQuadra(cartas, total)) return 7;
    if (temFullHouse(cartas, total)) return 6;
    if (temFlush(cartas, total)) return 5;
    if (temStraight(cartas, total)) return 4;
    if (temTrinca(cartas, total)) return 3;
    if (temDoisPares(cartas, total)) return 2;
    if (temPar(cartas, total)) return 1;
    *high_card = cartaAlta(cartas, total);
    return 0;
}

int pontuacaoMao(valor_carta *cartas, int total) {
    if (temFullHouse(cartas, total)) return 6;
    if (temFlush(cartas, total)) return 5;
    if (temTrinca(cartas, total)) return 4;
    if (temDoisPares(cartas, total)) return 3;
    if (temPar(cartas, total)) return 2;
    return 1;
}

#endif
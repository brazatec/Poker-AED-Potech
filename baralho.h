#ifndef baralho_h
#define baralho_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>

#include "disco.h"
#include "pontuacao.h"

#define CARTAS 52
#define CARTAS_POR_JOGADOR 2
#define MAX_JOGADORES 5
#define MIN_JOGADORES 2
#define SMALL_BLIND 10
#define BIG_BLIND 20
#define MAX 52

typedef struct {
    valor_carta cartas[CARTAS];
} baralho;

typedef struct {
    char nome[50];
    valor_carta *mao;
    int aposta;
    int aindaNaRodada;
    int fichas;
    int saldo;
} Jogador;

typedef struct {
    char jogador[50];
    char acao[20];
    int valor;
} Jogada;

void delay(int seconds) {
    sleep(seconds);
}

void printCarta(valor_carta carta) {
    char *cor;
    if (carta.naipe == L'♥' || carta.naipe == L'♦') {
        cor = "\033[1;31m";
    } else {
        cor = "\033[30m";
    }
    if (carta.royal != '0')
        printf("[\033[0m%c%s%lc\033[0m] ", carta.royal, cor, carta.naipe);
    else
        printf("[\033[0m%d%s%lc\033[0m] ", carta.valor, cor, carta.naipe);
}

void animarEmbaralhamento() {
    printf("\n\033[1;36m>>> Embaralhando cartas...\033[0m\n");
    char *frames[] = {
        "[\033[30m♠\033[0m \033[1;31m♥\033[0m \033[1;31m♦\033[0m \033[30m♣\033[0m \033[30m♠\033[0m \033[1;31m♥\033[0m \033[1;31m♦\033[0m \033[30m♣\033[0m]",
        "[\033[1;31m♥\033[0m \033[1;31m♦\033[0m \033[30m♣\033[0m \033[30m♠\033[0m \033[30m♣\033[0m \033[1;31m♦\033[0m \033[30m♠\033[0m \033[1;31m♥\033[0m]",
        "[\033[30m♣\033[0m \033[30m♠\033[0m \033[1;31m♥\033[0m \033[1;31m♦\033[0m \033[30m♠\033[0m \033[30m♣\033[0m \033[1;31m♥\033[0m \033[1;31m♦\033[0m]",
        "[\033[1;31m♦\033[0m \033[30m♣\033[0m \033[30m♠\033[0m \033[1;31m♥\033[0m \033[1;31m♦\033[0m \033[1;31m♥\033[0m \033[30m♣\033[0m \033[30m♠\033[0m]",
        "[\033[30m♠\033[0m \033[1;31m♦\033[0m \033[30m♣\033[0m \033[1;31m♥\033[0m \033[30m♠\033[0m \033[1;31m♦\033[0m \033[30m♣\033[0m \033[1;31m♥\033[0m]",
        "[\033[30m♠\033[0m \033[1;31m♥\033[0m \033[1;31m♦\033[0m \033[30m♣\033[0m \033[30m♠\033[0m \033[1;31m♥\033[0m \033[1;31m♦\033[0m \033[30m♣\033[0m]"
    };
    for (int i = 0; i < 12; i++) {
        printf("\r%s", frames[i % 6]);
        fflush(stdout);
        usleep(200000);
    }
    printf("\n\033[1;32mBaralho pronto!\033[0m\n");
    delay(1);
}

void animarCartaNaMesa(valor_carta c, int pos) {
    const int espacamento = pos * 6;
    const char *cor;
    if (c.naipe == L'♥' || c.naipe == L'♦')
        cor = "\033[1;31m";
    else
        cor = "\033[30m";
    for (int i = 0; i <= espacamento; i += 2) {
        printf("\r%*s", i, "");
        if (c.royal != '0') {
            printf("[%c%s%lc\033[0m] ", c.royal, cor, c.naipe);
        } else {
            printf("[%d%s%lc\033[0m] ", c.valor, cor, c.naipe);
        }
        fflush(stdout);
        usleep(100000);
    }
    printf("\n");
}

void exibirMesa(valor_carta *mesa) {
    printf("\n\033[1;34m========== CARTAS DA MESA ==========\033[0m\n");
    for (int i = 0; i < 5; i++)
        animarCartaNaMesa(mesa[i], i);
    printf("\033[1;34m====================================\033[0m\n");
}

void criarBaralho(baralho *baralho) {
    int posicaoVet = 0;
    for (int naipe = 1; naipe <= 4; naipe++) {
        for (int valor = 2; valor <= 14; valor++) {
            if (valor > 1 && valor < 11) {
                baralho->cartas[posicaoVet].valor = valor;
                baralho->cartas[posicaoVet].royal = '0';
            } else if (valor == 11) {
                baralho->cartas[posicaoVet].royal = 'J';
            } else if (valor == 12) {
                baralho->cartas[posicaoVet].royal = 'Q';
            } else if (valor == 13) {
                baralho->cartas[posicaoVet].royal = 'K';
            } else if (valor == 14) {
                baralho->cartas[posicaoVet].royal = 'A';
            }
            if (naipe == 1) baralho->cartas[posicaoVet].naipe = L'♣';
            else if (naipe == 2) baralho->cartas[posicaoVet].naipe = L'♦';
            else if (naipe == 3) baralho->cartas[posicaoVet].naipe = L'♥';
            else if (naipe == 4) baralho->cartas[posicaoVet].naipe = L'♠';
            baralho->cartas[posicaoVet].valor = valor;
            posicaoVet++;
        }
    }
}

void embaralharBaralho(baralho *baralho) {
    srand(time(NULL));
    for (int i = CARTAS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        valor_carta aux = baralho->cartas[i];
        baralho->cartas[i] = baralho->cartas[j];
        baralho->cartas[j] = aux;
    }
}

void distribuirCartas(Jogador jogadores[], int num_jogadores, valor_carta **mesa, tp_pilha *monte) {
    for (int i = 0; i < num_jogadores; i++) {
        jogadores[i].mao = malloc(CARTAS_POR_JOGADOR * sizeof(valor_carta));
        for (int j = 0; j < CARTAS_POR_JOGADOR; j++) {
            pop(monte, &jogadores[i].mao[j]);
        }
    }
    *mesa = malloc(5 * sizeof(valor_carta));
    for (int i = 0; i < 5; i++) {
        pop(monte, &(*mesa)[i]);
    }
}

#endif

#ifndef disco_h
#define disco_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>

// ModularizaC'C#o
#include "baralho.h"
#include "pontuacao.h"

#define CARTAS 52
#define CARTAS_POR_JOGADOR 2
#define MAX_JOGADORES 5
#define MIN_JOGADORES 2
#define SMALL_BLIND 10
#define BIG_BLIND 20
#define MAX 52

void salvarJogadaEmDisco(Jogada jogada) {
	FILE *file = fopen("jogadas.txt", "a");
	if (file == NULL) {
		printf("Erro ao abrir arquivo para escrita!\n");
		return;
	}
	fprintf(file, "Jogador: %s, AC'C#o: %s, Valor: %d\n", jogada.jogador, jogada.acao, jogada.valor);
	fclose(file);
}

void exibirJogadasDoDisco() {
	FILE *file = fopen("jogadas.txt", "r");
	if (file == NULL) {
		printf("Nenhuma jogada registrada em disco.\n");
		return;
	}
	printf("\n\033[1;36m=== HistC3rico de Jogadas ===\033[0m\n");
	char linha[100];
	while (fgets(linha, sizeof(linha), file)) {
		printf("%s", linha);
	}
	fclose(file);
}

#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>

#include "pilha.h"
#include "arvoreAVL.h"

// Modularização
#include "disco.h"
#include "baralho.h"
#include "pontuacao.h"

#define CARTAS 52
#define CARTAS_POR_JOGADOR 2
#define MAX_JOGADORES 5
#define MIN_JOGADORES 2
#define SMALL_BLIND 10
#define BIG_BLIND 20
#define MAX 52

void limparTela() {
    printf("\033[H\033[J");
}

void limparLinha() {
    printf("\033[2K\033[1G");
}

void aguardarEnter() {
    printf("Pressione Enter para continuar...\n");
    while (getchar() != '\n');
}

void exibirMaoJogador(Jogador *jogador, valor_carta *mesa) {
    printf("Mão: ");
    for (int j = 0; j < CARTAS_POR_JOGADOR; j++)
        printCarta(jogador->mao[j]);
    printf("\n");
    valor_carta combinadas[7];
    for (int j = 0; j < CARTAS_POR_JOGADOR; j++)
        combinadas[j] = jogador->mao[j];
    for (int j = 0; j < 5; j++)
        combinadas[j + CARTAS_POR_JOGADOR] = mesa[j];
    printf("Combinação: ");
    if (temFullHouse(combinadas, 7)) printf("Full House\n");
    else if (temFlush(combinadas, 7)) printf("Flush\n");
    else if (temTrinca(combinadas, 7)) printf("Trinca\n");
    else if (temDoisPares(combinadas, 7)) printf("Dois Pares\n");
    else if (temPar(combinadas, 7)) printf("Um Par\n");
    else printf("Carta Alta: %d\n", cartaAlta(combinadas, 7));
}

void exibirMesaParcial(valor_carta *mesa, int num_cartas) {
    printf("\n\033[1;34m========== CARTAS DA MESA ==========\033[0m\n");
    for (int i = 0; i < num_cartas; i++)
        animarCartaNaMesa(mesa[i], i);
    printf("\033[1;34m====================================\033[0m\n");
}

void exibirMaoJogadorCompleta(Jogador *jogador, valor_carta *mesa, int num_cartas_mesa) {
    printf("Mão: ");
    for (int j = 0; j < CARTAS_POR_JOGADOR; j++)
        printCarta(jogador->mao[j]);
    printf("\n");
    valor_carta combinadas[7];
    int total_cartas = CARTAS_POR_JOGADOR + num_cartas_mesa;
    for (int j = 0; j < CARTAS_POR_JOGADOR; j++)
        combinadas[j] = jogador->mao[j];
    for (int j = 0; j < num_cartas_mesa; j++)
        combinadas[j + CARTAS_POR_JOGADOR] = mesa[j];
    int high_card = 0;
    printf("Combinação: ");
    int pontos = pontuacaoMaoCompleta(combinadas, total_cartas, &high_card);
    switch (pontos) {
        case 9: printf("Royal Flush\n"); break;
        case 8: printf("Straight Flush\n"); break;
        case 7: printf("Quadra\n"); break;
        case 6: printf("Full House\n"); break;
        case 5: printf("Flush\n"); break;
        case 4: printf("Straight\n"); break;
        case 3: printf("Trinca\n"); break;
        case 2: printf("Dois Pares\n"); break;
        case 1: printf("Um Par\n"); break;
        default: printf("Carta Alta: %d\n", high_card);
    }
}

void postarBlinds(Jogador jogadores[], int num_jogadores, int dealer, int *pote, ArvAVL *arvoreJogadas) {
    int sb_idx = (dealer + 1) % num_jogadores;
    int bb_idx = (dealer + 2) % num_jogadores;
    while (!jogadores[sb_idx].aindaNaRodada && jogadores[sb_idx].saldo <= 0)
        sb_idx = (sb_idx + 1) % num_jogadores;
    while (!jogadores[bb_idx].aindaNaRodada && jogadores[bb_idx].saldo <= 0)
        bb_idx = (bb_idx + 1) % num_jogadores;

    int sb_amount = (jogadores[sb_idx].saldo >= SMALL_BLIND) ? SMALL_BLIND : jogadores[sb_idx].saldo;
    jogadores[sb_idx].saldo -= sb_amount;
    jogadores[sb_idx].fichas -= sb_amount;
    jogadores[sb_idx].aposta += sb_amount;
    *pote += sb_amount;
    Jogada jogada_sb = { .jogador = "", .acao = "Small Blind", .valor = sb_amount };
    strcpy(jogada_sb.jogador, jogadores[sb_idx].nome);
    salvarJogadaEmDisco(jogada_sb);
    inserir(arvoreJogadas, rand());

    int bb_amount = (jogadores[bb_idx].saldo >= BIG_BLIND) ? BIG_BLIND : jogadores[bb_idx].saldo;
    jogadores[bb_idx].saldo -= bb_amount;
    jogadores[bb_idx].fichas -= bb_amount;
    jogadores[bb_idx].aposta += bb_amount;
    *pote += bb_amount;
    Jogada jogada_bb = { .jogador = "", .acao = "Big Blind", .valor = bb_amount };
    strcpy(jogada_bb.jogador, jogadores[bb_idx].nome);
    salvarJogadaEmDisco(jogada_bb);
    inserir(arvoreJogadas, rand());
}

// Função para limpar completamente a tela do terminal
void limparTelaCompleta() {
    printf("\033[H\033[J");
}

void rodadaDeApostas(Jogador jogadores[], int num_jogadores, valor_carta *mesa, ArvAVL *arvoreJogadas) {
    exibirMesa(mesa);
    printf("\n\033[1;36m>>> Iniciando a rodada de apostas...\033[0m\n");
    int apostaAtual = 0;
    int pote = 0;
    for (int i = 0; i < num_jogadores; i++) {
        jogadores[i].aindaNaRodada = 1;
        jogadores[i].aposta = 0;
        jogadores[i].fichas = jogadores[i].saldo;
    }
    int jogadoresAtivos;
    int houveAumento;
    do {
        jogadoresAtivos = 0;
        houveAumento = 0;
        for (int i = 0; i < num_jogadores; i++) {
            if (!jogadores[i].aindaNaRodada)
                continue;
            if (jogadores[i].aposta < apostaAtual || apostaAtual == 0) {
                printf("\n\033[1;33m>> Jogador: %s, pressione Enter para ver suas cartas.\033[0m\n", jogadores[i].nome);
                aguardarEnter();
                exibirMaoJogador(&jogadores[i], mesa);
                printf("\n%s, você tem %d fichas.\n", jogadores[i].nome, jogadores[i].fichas);
                printf("Aposta atual: %d \nSua aposta: %d \nPote: %d\n", apostaAtual, jogadores[i].aposta, pote);
                int escolha;
                Jogada jogada;
                strcpy(jogada.jogador, jogadores[i].nome);
                jogada.valor = 0;
                if (apostaAtual == 0) {
                    printf("Escolha:\n[1] Check\n[2] Raise\n[3] Fold\n> ");
                    scanf("%d", &escolha);
                    while (getchar() != '\n');
                    if (escolha == 1) {
                        strcpy(jogada.acao, "Check");
                        printf("%s deu check.\n", jogadores[i].nome);
                        salvarJogadaEmDisco(jogada);
                        inserir(arvoreJogadas, rand());
                    }
                } else {
                    printf("Escolha:\n[1] Call\n[2] Raise\n[3] Fold\n> ");
                    scanf("%d", &escolha);
                    while (getchar() != '\n');
                    if (escolha == 1) {
                        int pagar = apostaAtual - jogadores[i].aposta;
                        if (pagar > jogadores[i].fichas) pagar = jogadores[i].fichas;
                        jogadores[i].fichas -= pagar;
                        jogadores[i].aposta += pagar;
                        pote += pagar;
                        strcpy(jogada.acao, "Call");
                        jogada.valor = pagar;
                        salvarJogadaEmDisco(jogada);
                        inserir(arvoreJogadas, rand());
                    }
                }
                if (escolha == 2) {
                    int aumento;
                    printf("Digite o valor total que deseja apostar: ");
                    scanf("%d", &aumento);
                    while (getchar() != '\n');
                    if (aumento <= apostaAtual) {
                        printf("Você deve apostar mais que %d!\n", apostaAtual);
                        i--;
                        continue;
                    }
                    int valor = aumento - jogadores[i].aposta;
                    if (valor > jogadores[i].fichas) valor = jogadores[i].fichas;
                    jogadores[i].fichas -= valor;
                    jogadores[i].aposta += valor;
                    apostaAtual = jogadores[i].aposta;
                    pote += valor;
                    houveAumento = 1;
                    strcpy(jogada.acao, "Raise");
                    jogada.valor = aumento;
                    salvarJogadaEmDisco(jogada);
                    inserir(arvoreJogadas, rand());
                }
                else if (escolha == 3) {
                    jogadores[i].aindaNaRodada = 0;
                    printf("%s desistiu da rodada.\n", jogadores[i].nome);
                    strcpy(jogada.acao, "Fold");
                    salvarJogadaEmDisco(jogada);
                    inserir(arvoreJogadas, rand());
                }
                printf("\033[1;33mPressione Enter para ocultar suas cartas e passar a vez.\033[0m\n");
                aguardarEnter();
                for (int j = 0; j < 20; j++) {
                    printf("\033[1A\033[2K");
                }
            }
        }
        for (int i = 0; i < num_jogadores; i++) {
            if (jogadores[i].aindaNaRodada && jogadores[i].aposta < apostaAtual)
                jogadoresAtivos++;
        }
    } while (houveAumento || jogadoresAtivos > 0);
    printf("\n\033[1;32mPote total: %d fichas.\033[0m\n", pote);
}

void determinarVencedor(Jogador jogadores[], int num_jogadores, valor_carta *mesa) {
    printf("\n\033[1;34m>>> Showdown: Exibindo mãos dos jogadores ativos <<<\033[0m\n");
    for (int i = 0; i < num_jogadores; i++) {
        if (jogadores[i].aindaNaRodada) {
            printf("\nJogador: %s\n", jogadores[i].nome);
            exibirMaoJogador(&jogadores[i], mesa);
        }
    }
    int maiorPontuacao = 0;
    int vencedores[MAX_JOGADORES] = {0};
    int num_vencedores = 0;
    int pote = 0;
    for (int i = 0; i < num_jogadores; i++) {
        pote += jogadores[i].aposta;
        if (jogadores[i].aindaNaRodada) {
            valor_carta combinadas[7];
            for (int j = 0; j < CARTAS_POR_JOGADOR; j++)
                combinadas[j] = jogadores[i].mao[j];
            for (int j = 0; j < 5; j++)
                combinadas[j + CARTAS_POR_JOGADOR] = mesa[j];
            int pontos = pontuacaoMao(combinadas, 7);
            if (pontos > maiorPontuacao) {
                maiorPontuacao = pontos;
                num_vencedores = 0;
                vencedores[num_vencedores++] = i;
            } else if (pontos == maiorPontuacao) {
                vencedores[num_vencedores++] = i;
            }
        }
    }
    printf("\n\033[1;32m>>> Resultado Final <<<\033[0m\n");
    if (num_vencedores == 1) {
        printf("O vencedor é: \033[1;33m%s\033[0m! Ele ganha o pote de %d.\n", jogadores[vencedores[0]].nome, pote);
        jogadores[vencedores[0]].saldo += pote;
    } else {
        printf("Empate entre: ");
        for (int i = 0; i < num_vencedores; i++) {
            printf("%s ", jogadores[vencedores[i]].nome);
        }
        printf("\nCada um recebe %d.\n", pote / num_vencedores);
        for (int i = 0; i < num_vencedores; i++) {
            jogadores[vencedores[i]].saldo += pote / num_vencedores;
        }
    }
}

//

void rodadaDeApostasCompleta(Jogador jogadores[], int num_jogadores, valor_carta *mesa, int num_cartas_mesa, ArvAVL *arvoreJogadas, int dealer, int *pote) {
    exibirMesaParcial(mesa, num_cartas_mesa);
    printf("\n\033[1;36m>>> Iniciando a rodada de apostas...\033[0m\n");

    int apostaAtual = (num_cartas_mesa == 0) ? BIG_BLIND : 0;
    int start_idx = (num_cartas_mesa == 0) ? (dealer + 3) % num_jogadores : (dealer + 1) % num_jogadores;

    int jogadoresAtivos;
    int houveAumento;
    do {
        jogadoresAtivos = 0;
        houveAumento = 0;
        for (int i = 0; i < num_jogadores; i++) {
            int idx = (start_idx + i) % num_jogadores;
            if (!jogadores[idx].aindaNaRodada)
                continue;
            if (jogadores[idx].aposta < apostaAtual || apostaAtual == 0) {
                printf("\n\033[1;33m>> Jogador: %s, pressione Enter para ver suas cartas.\033[0m\n", jogadores[idx].nome);
                aguardarEnter();
                exibirMaoJogadorCompleta(&jogadores[idx], mesa, num_cartas_mesa);
                printf("\n%s, você tem %d fichas.\n", jogadores[idx].nome, jogadores[idx].fichas);
                printf("Aposta atual: %d \nSua aposta: %d \nPote: %d\n", apostaAtual, jogadores[idx].aposta, *pote);
                int escolha;
                Jogada jogada = { .jogador = "", .acao = "", .valor = 0 };
                strcpy(jogada.jogador, jogadores[idx].nome);
                if (apostaAtual == 0) {
                    printf("Escolha:\n[1] Check\n[2] Raise\n[3] Fold\n> ");
                    scanf("%d", &escolha);
                    while (getchar() != '\n');
                    if (escolha == 1) {
                        strcpy(jogada.acao, "Check");
                        printf("%s deu check.\n", jogadores[idx].nome);
                        salvarJogadaEmDisco(jogada);
                        inserir(arvoreJogadas, rand());
                    }
                } else {
                    printf("Escolha:\n[1] Call\n[2] Raise\n[3] Fold\n> ");
                    scanf("%d", &escolha);
                    while (getchar() != '\n');
                    if (escolha == 1) {
                        int pagar = apostaAtual - jogadores[idx].aposta;
                        if (pagar > jogadores[idx].fichas) pagar = jogadores[idx].fichas;
                        jogadores[idx].fichas -= pagar;
                        jogadores[idx].aposta += pagar;
                        jogadores[idx].saldo -= pagar;
                        *pote += pagar;
                        strcpy(jogada.acao, "Call");
                        jogada.valor = pagar;
                        salvarJogadaEmDisco(jogada);
                        inserir(arvoreJogadas, rand());
                    }
                }
                if (escolha == 2) {
                    int aumento;
                    printf("Digite o valor total que deseja apostar: ");
                    scanf("%d", &aumento);
                    while (getchar() != '\n');
                    if (aumento <= apostaAtual) {
                        printf("Você deve apostar mais que %d!\n", apostaAtual);
                        i--;
                        continue;
                    }
                    if (aumento > jogadores[idx].fichas + jogadores[idx].aposta) {
                        printf("Você não tem fichas suficientes!\n");
                        i--;
                        continue;
                    }
                    int valor = aumento - jogadores[idx].aposta;
                    jogadores[idx].fichas -= valor;
                    jogadores[idx].saldo -= valor;
                    jogadores[idx].aposta = aumento;
                    apostaAtual = aumento;
                    *pote += valor;
                    houveAumento = 1;
                    strcpy(jogada.acao, "Raise");
                    jogada.valor = aumento;
                    salvarJogadaEmDisco(jogada);
                    inserir(arvoreJogadas, rand());
                } else if (escolha == 3) {
                    jogadores[idx].aindaNaRodada = 0;
                    printf("%s desistiu da rodada.\n", jogadores[idx].nome);
                    strcpy(jogada.acao, "Fold");
                    salvarJogadaEmDisco(jogada);
                    inserir(arvoreJogadas, rand());
                }
                printf("\033[1;33mPressione Enter para ocultar suas cartas e passar a vez.\033[0m\n");
                aguardarEnter();
                for (int j = 0; j < 20; j++) {
                    printf("\033[1A\033[2K");
                }
            }
            if (jogadores[idx].aindaNaRodada)
                jogadoresAtivos++;
        }
    } while (houveAumento || jogadoresAtivos > 1);
    printf("\n\033[1;32mPote total: %d fichas.\033[0m\n", *pote);
}

void determinarVencedorCompleto(Jogador jogadores[], int num_jogadores, valor_carta *mesa, int pote) {
    int jogadores_ativos = 0;
    for (int i = 0; i < num_jogadores; i++) {
        if (jogadores[i].aindaNaRodada) jogadores_ativos++;
    }
    if (jogadores_ativos == 1) {
        for (int i = 0; i < num_jogadores; i++) {
            if (jogadores[i].aindaNaRodada) {
                printf("\n\033[1;32m>>> %s vence por desistência dos outros jogadores! Ganha %d fichas.\033[0m\n", jogadores[i].nome, pote);
                jogadores[i].saldo += pote;
                return;
            }
        }
    }
    printf("\n\033[1;34m>>> Showdown: Exibindo mãos dos jogadores ativos <<<\033[0m\n");
    for (int i = 0; i < num_jogadores; i++) {
        if (jogadores[i].aindaNaRodada) {
            printf("\nJogador: %s\n", jogadores[i].nome);
            exibirMaoJogadorCompleta(&jogadores[i], mesa, 5);
        }
    }
    int maiorPontuacao = -1;
    int high_card = 0;
    int vencedores[MAX_JOGADORES] = {0};
    int num_vencedores = 0;
    for (int i = 0; i < num_jogadores; i++) {
        if (jogadores[i].aindaNaRodada) {
            valor_carta combinadas[7];
            for (int j = 0; j < CARTAS_POR_JOGADOR; j++)
                combinadas[j] = jogadores[i].mao[j];
            for (int j = 0; j < 5; j++)
                combinadas[j + CARTAS_POR_JOGADOR] = mesa[j];
            int pontos = pontuacaoMaoCompleta(combinadas, 7, &high_card);
            if (pontos > maiorPontuacao) {
                maiorPontuacao = pontos;
                num_vencedores = 0;
                vencedores[num_vencedores++] = i;
            } else if (pontos == maiorPontuacao) {
                vencedores[num_vencedores++] = i;
            }
        }
    }
    printf("\n\033[1;32m>>> Resultado Final <<<\033[0m\n");
    if (num_vencedores == 1) {
        printf("O vencedor é: \033[1;33m%s\033[0m! Ele ganha o pote de %d.\n", jogadores[vencedores[0]].nome, pote);
        jogadores[vencedores[0]].saldo += pote;
    } else {
        printf("Empate entre: ");
        for (int i = 0; i < num_vencedores; i++) {
            printf("%s ", jogadores[vencedores[i]].nome);
        }
        printf("\nCada um recebe %d.\n", pote / num_vencedores);
        for (int i = 0; i < num_vencedores; i++) {
            jogadores[vencedores[i]].saldo += pote / num_vencedores;
        }
    }
}


int main() {
    setlocale(LC_ALL, "");

    int num_jogadores;
    do {
        printf("Digite o número de jogadores (entre %d e %d): ", MIN_JOGADORES, MAX_JOGADORES);
        scanf("%d", &num_jogadores);
        while (getchar() != '\n');
    } while (num_jogadores < MIN_JOGADORES || num_jogadores > MAX_JOGADORES);

    Jogador jogadores[num_jogadores];
    for (int i = 0; i < num_jogadores; i++) {
        printf("Digite o nome do jogador %d: ", i + 1);
        scanf(" %49s", jogadores[i].nome);
        while (getchar() != '\n');
        jogadores[i].saldo = 1000;
        jogadores[i].aposta = 0;
    }

    char continuar;
    do {
        // Criar e embaralhar baralho
        baralho baralhoDeCartas;
        criarBaralho(&baralhoDeCartas);
        embaralharBaralho(&baralhoDeCartas);
        animarEmbaralhamento();

        tp_pilha pilhaCartas;
        inicializaPilha(&pilhaCartas);
        for (int i = 0; i < CARTAS; i++)
            push(&pilhaCartas, baralhoDeCartas.cartas[i]);

        valor_carta *mesa;
        distribuirCartas(jogadores, num_jogadores, &mesa, &pilhaCartas);

        ArvAVL *arvoreJogadas = criarAVL();

        rodadaDeApostas(jogadores, num_jogadores, mesa, arvoreJogadas);
        determinarVencedor(jogadores, num_jogadores, mesa);
        exibirJogadasDoDisco();

        for (int i = 0; i < num_jogadores; i++)
            free(jogadores[i].mao);
        free(mesa);
        liberaABB(arvoreJogadas);

        int jogadoresComFichas = 0;
        for (int i = 0; i < num_jogadores; i++) {
            if (jogadores[i].saldo > 0)
                jogadoresComFichas++;
        }
        if (jogadoresComFichas < 2) {
            printf("\nJogo encerrado. Menos de dois jogadores com fichas.\n");
            break;
        }

        printf("\nDeseja jogar outra rodada? (s/n): ");
        scanf(" %c", &continuar);
        while (getchar() != '\n');

    } while (continuar == 's' || continuar == 'S');

    printf("\n\033[1;32mJogo encerrado. Obrigado por jogar!\033[0m\n");
    return 0;
}
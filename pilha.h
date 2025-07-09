#ifndef pilha_h
#define pilha_h
#define MAX 52

typedef struct {
    int valor;  
    wchar_t naipe;
    char royal; 
} valor_carta; // Objeto de uma carta, que pode conter numeros e naipes, ou letras royals e naipes

typedef valor_carta tp_item; // tp_item recebe o tipo do objeto da carta

typedef struct {
    int topo; 
    tp_item item[MAX]; 
} tp_pilha;

void inicializaPilha(tp_pilha *p) {
    p->topo = -1;
}

int pilhaVazia(tp_pilha *p) {
    return (p->topo == -1);
}

int pilhaCheia(tp_pilha *p) {
    return (p->topo == MAX - 1);
}

int push(tp_pilha *p, tp_item e) {
    if (pilhaCheia(p)) {
        return 0;
    }
    p->topo++; 
    p->item[p->topo] = e; 
    return 1;
}

int pop(tp_pilha *p, tp_item *e) {
    if (pilhaVazia(p)) {
        return 0;
    }
    *e = p->item[p->topo]; 
    p->topo--; 
    return 1;
}

void imprimePilha(tp_pilha p) {
    tp_item e;
    printf("\n");
    while (!pilhaVazia(&p)) { 
        pop(&p, &e);
        if (e.royal != '0') {
            printf("%c%lc \n", e.royal, e.naipe);  
        } else {
            printf("%d%lc \n", e.valor, e.naipe);  
        }
    }
    printf("\n");
}

int alturaPilha(tp_pilha *p){
    return p->topo++;
}

#endif

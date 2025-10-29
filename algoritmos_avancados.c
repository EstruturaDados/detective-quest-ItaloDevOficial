#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------
   Estruturas de dados
   ------------------------*/

/* Nó da árvore que representa uma sala da mansão */
typedef struct sala {
    char nome[50];
    char pista[100];       /* pode estar vazia "" se não tiver pista */
    int pistaColetada;     /* 0 = não coletada ainda, 1 = coletada */
    struct sala *esquerda;
    struct sala *direita;
} Sala;

/* Nó da BST que armazena as pistas coletadas */
typedef struct pistaNode {
    char pista[100];
    struct pistaNode *esq;
    struct pistaNode *dir;
} PistaNode;

/* ------------------------
   Protótipos
   ------------------------*/
Sala* criarSala(const char nome[], const char pista[]);
PistaNode* inserirPista(PistaNode *raiz, const char *pista);
void exibirPistas(PistaNode *raiz);
void explorarSalasComPistas(Sala *inicio, PistaNode **raizPistas);
void liberarSalas(Sala *raiz);
void liberarPistas(PistaNode *raiz);

/* ------------------------
   Implementações
   ------------------------*/

/* Cria dinamicamente uma sala com nome e pista (pista pode ser "" se não existir) */
Sala* criarSala(const char nome[], const char pista[]) {
    Sala *nova = (Sala*) malloc(sizeof(Sala));
    if (!nova) {
        fprintf(stderr, "Erro de alocação de memória para Sala.\n");
        exit(EXIT_FAILURE);
    }
    strncpy(nova->nome, nome, sizeof(nova->nome)-1);
    nova->nome[sizeof(nova->nome)-1] = '\0';
    strncpy(nova->pista, pista, sizeof(nova->pista)-1);
    nova->pista[sizeof(nova->pista)-1] = '\0';
    nova->pistaColetada = 0;
    nova->esquerda = NULL;
    nova->direita = NULL;
    return nova;
}

/* Insere uma nova pista na BST (alfabeticamente). Evita inserir duplicatas exatas. */
PistaNode* inserirPista(PistaNode *raiz, const char *pista) {
    if (pista == NULL || pista[0] == '\0') return raiz; /* nada a inserir */

    if (raiz == NULL) {
        PistaNode *no = (PistaNode*) malloc(sizeof(PistaNode));
        if (!no) {
            fprintf(stderr, "Erro de alocação de memória para PistaNode.\n");
            exit(EXIT_FAILURE);
        }
        strncpy(no->pista, pista, sizeof(no->pista)-1);
        no->pista[sizeof(no->pista)-1] = '\0';
        no->esq = no->dir = NULL;
        return no;
    }

    int cmp = strcmp(pista, raiz->pista);
    if (cmp < 0) {
        raiz->esq = inserirPista(raiz->esq, pista);
    } else if (cmp > 0) {
        raiz->dir = inserirPista(raiz->dir, pista);
    } else {
        /* pista igual já presente -> não inserir duplicata */
    }
    return raiz;
}

/* Percorre a BST em ordem (in-order) e imprime as pistas em ordem alfabética */
void exibirPistas(PistaNode *raiz) {
    if (raiz == NULL) return;
    exibirPistas(raiz->esq);
    printf("- %s\n", raiz->pista);
    exibirPistas(raiz->dir);
}

/* Função que controla a navegação do jogador e coleta automática de pistas */
void explorarSalasComPistas(Sala *inicio, PistaNode **raizPistas) {
    if (!inicio) return;

    Sala *atual = inicio;
    char escolha;

    printf("Bem-vindo ao Detective Quest (coleção de pistas)!\n");

    while (1) {
        printf("\nVocê está na: %s\n", atual->nome);

        /* Se houver pista e ela não tiver sido coletada ainda, coleta e insere na BST */
        if (atual->pista[0] != '\0' && !atual->pistaColetada) {
            printf("Pista encontrada: \"%s\"\n", atual->pista);
            *raizPistas = inserirPista(*raizPistas, atual->pista);
            atual->pistaColetada = 1;
            printf("Pista adicionada ao caderno de investigação.\n");
        } else if (atual->pista[0] != '\0' && atual->pistaColetada) {
            printf("Você já coletou a pista desta sala anteriormente.\n");
        } else {
            printf("Não há pista nesta sala.\n");
        }

        /* Mostra opções */
        printf("\nEscolha um caminho:\n");
        printf(" (e) Ir para a esquerda\n");
        printf(" (d) Ir para a direita\n");
        printf(" (s) Sair da exploração e ver pistas coletadas\n");
        printf("Sua escolha: ");
        if (scanf(" %c", &escolha) != 1) {
            /* leitura falhou; limpar stdin e repetir */
            int c;
            while ((c = getchar()) != EOF && c != '\n') ;
            printf("Entrada inválida. Tente novamente.\n");
            continue;
        }

        if (escolha == 'e' || escolha == 'E') {
            if (atual->esquerda != NULL) {
                atual = atual->esquerda;
            } else {
                printf("Não há sala à esquerda. Escolha outro caminho.\n");
            }
        } else if (escolha == 'd' || escolha == 'D') {
            if (atual->direita != NULL) {
                atual = atual->direita;
            } else {
                printf("Não há sala à direita. Escolha outro caminho.\n");
            }
        } else if (escolha == 's' || escolha == 'S') {
            printf("\nVocê optou por encerrar a exploração.\n");
            return;
        } else {
            printf("Opção inválida! Use 'e', 'd' ou 's'.\n");
        }
    }
}

/* Funções auxiliares para liberar memória (boas práticas) */
void liberarSalas(Sala *raiz) {
    if (!raiz) return;
    liberarSalas(raiz->esquerda);
    liberarSalas(raiz->direita);
    free(raiz);
}

void liberarPistas(PistaNode *raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esq);
    liberarPistas(raiz->dir);
    free(raiz);
}

/* ------------------------
   main: monta a mansão e inicia a exploração
   ------------------------*/
int main() {
    /* Montagem fixa da árvore (mesma mansão do exemplo anterior) */
    Sala *hall = criarSala("Hall de Entrada", "Envelope com um símbolo estranho");
    hall->esquerda = criarSala("Sala de Estar", "Pegadas de lama perto do tapete");
    hall->direita = criarSala("Biblioteca", "Página rasgada de diário");

    hall->esquerda->esquerda = criarSala("Cozinha", "Chave dourada enferrujada");
    hall->esquerda->direita  = criarSala("Sala de Jantar", "Guardanapo com anotação");
    
    hall->direita->esquerda  = criarSala("Quarto de Hóspedes", "Meia caída sob a cama");
    hall->direita->direita   = criarSala("Jardim Interno", "Pegadas que levam para o muro");

    /* raiz da BST de pistas (inicialmente vazia) */
    PistaNode *raizPistas = NULL;

    /* Inicia exploração e coleta de pistas */
    explorarSalasComPistas(hall, &raizPistas);

    /* Exibe todas as pistas coletadas em ordem alfabética */
    printf("\n--- Pistas coletadas (em ordem alfabética) ---\n");
    if (raizPistas == NULL) {
        printf("Nenhuma pista foi coletada.\n");
    } else {
        exibirPistas(raizPistas);
    }

    /* Liberar memória alocada antes de encerrar */
    liberarSalas(hall);
    liberarPistas(raizPistas);

    printf("\nObrigado por jogar Detective Quest!\n");
    return 0;
}

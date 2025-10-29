#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXNOME 50
#define MAXPISTA 100
#define HASH_SIZE 101  /* número primo pequeno para a hash table */

/* ------------------------
   Estruturas de dados
   ------------------------*/

/* Nó da árvore que representa uma sala da mansão */
typedef struct sala {
    char nome[MAXNOME];
    char pista[MAXPISTA];     /* pode estar vazia "" se não houver pista */
    int pistaColetada;        /* 0 = não coletada, 1 = coletada */
    struct sala *esquerda;
    struct sala *direita;
} Sala;

/* Nó da BST que armazena as pistas coletadas */
typedef struct pistaNode {
    char pista[MAXPISTA];
    struct pistaNode *esq;
    struct pistaNode *dir;
} PistaNode;

/* Entrada para encadeamento na tabela hash: mapeia pista -> suspeito */
typedef struct hashEntry {
    char pista[MAXPISTA];
    char suspeito[MAXNOME];
    struct hashEntry *next;
} HashEntry;

/* ------------------------
   Protótipos
   ------------------------*/

/* Sala e BST */
Sala* criarSala(const char nome[], const char pista[]);   /* cria dinamicamente uma sala */
void explorarSalas(Sala *inicio, PistaNode **raizPistas); /* navega pela árvore e coleta pistas */

/* BST de pistas */
PistaNode* inserirPista(PistaNode *raiz, const char *pista); /* insere pista na BST */
void exibirPistas(PistaNode *raiz);                          /* exibe pistas em ordem (alfabética) */
void liberarPistas(PistaNode *raiz);

/* Hash */
unsigned long hash_djb2(const char *str);
void inserirNaHash(HashEntry *tabela[], const char *pista, const char *suspeito); /* insere associação pista->suspeito */
char* encontrarSuspeito(HashEntry *tabela[], const char *pista);                   /* encontra suspeito por pista */
void liberarHash(HashEntry *tabela[]);

/* Julgamento */
void verificarSuspeitoFinal(PistaNode *raizPistas, HashEntry *tabela[]); /* pede acusação e verifica evidências */

/* Limpeza das salas */
void liberarSalas(Sala *raiz);

/* Utilitários */
void limparBufferStdin(void);
void minusculas(char *s);

/* ------------------------
   Implementações
   ------------------------*/

/* Cria dinamicamente uma sala com nome e pista (pista pode ser "" se não existir)
   Comentário: criarSala() — cria dinamicamente um cômodo. */
Sala* criarSala(const char nome[], const char pista[]) {
    Sala *no = (Sala*) malloc(sizeof(Sala));
    if (!no) {
        fprintf(stderr, "Erro de alocação ao criar sala.\n");
        exit(EXIT_FAILURE);
    }
    strncpy(no->nome, nome, MAXNOME-1); no->nome[MAXNOME-1] = '\0';
    strncpy(no->pista, pista, MAXPISTA-1); no->pista[MAXPISTA-1] = '\0';
    no->pistaColetada = 0;
    no->esquerda = NULL;
    no->direita = NULL;
    return no;
}

/* Inserir pista na BST (ordenada alfabeticamente). Evita inserir string vazia.
   Comentário: inserirPista() / adicionarPista() — insere a pista coletada na árvore de pistas. */
PistaNode* inserirPista(PistaNode *raiz, const char *pista) {
    if (pista == NULL || pista[0] == '\0') return raiz;

    if (raiz == NULL) {
        PistaNode *no = (PistaNode*) malloc(sizeof(PistaNode));
        if (!no) { fprintf(stderr, "Erro alocação PistaNode\n"); exit(EXIT_FAILURE); }
        strncpy(no->pista, pista, MAXPISTA-1); no->pista[MAXPISTA-1] = '\0';
        no->esq = no->dir = NULL;
        return no;
    }
    int cmp = strcmp(pista, raiz->pista);
    if (cmp < 0) raiz->esq = inserirPista(raiz->esq, pista);
    else if (cmp > 0) raiz->dir = inserirPista(raiz->dir, pista);
    /* se for igual, já existe — não insere duplicata */
    return raiz;
}

/* In-order traversal: imprime as pistas em ordem alfabética */
void exibirPistas(PistaNode *raiz) {
    if (!raiz) return;
    exibirPistas(raiz->esq);
    printf("- %s\n", raiz->pista);
    exibirPistas(raiz->dir);
}

void liberarPistas(PistaNode *raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esq);
    liberarPistas(raiz->dir);
    free(raiz);
}

/* Função de hash djb2 */
unsigned long hash_djb2(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

/* Insere associação pista -> suspeito na tabela hash (encadeamento) 
   Comentário: inserirNaHash() — insere associação pista/suspeito na tabela hash. */
void inserirNaHash(HashEntry *tabela[], const char *pista, const char *suspeito) {
    if (!pista || pista[0] == '\0' || !suspeito) return;
    unsigned long h = hash_djb2(pista) % HASH_SIZE;
    /* verificar se já existe a mesma pista (atualiza suspeito se necessário) */
    HashEntry *cur = tabela[h];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0) {
            /* atualiza suspeito existente */
            strncpy(cur->suspeito, suspeito, MAXNOME-1); cur->suspeito[MAXNOME-1]='\0';
            return;
        }
        cur = cur->next;
    }
    /* inserir no início da lista encadeada */
    HashEntry *novo = (HashEntry*) malloc(sizeof(HashEntry));
    if (!novo) { fprintf(stderr, "Erro alocação HashEntry\n"); exit(EXIT_FAILURE); }
    strncpy(novo->pista, pista, MAXPISTA-1); novo->pista[MAXPISTA-1] = '\0';
    strncpy(novo->suspeito, suspeito, MAXNOME-1); novo->suspeito[MAXNOME-1] = '\0';
    novo->next = tabela[h];
    tabela[h] = novo;
}

/* Procura o suspeito associado a uma pista; retorna NULL se não encontrado.
   Comentário: encontrarSuspeito() — consulta o suspeito correspondente a uma pista. */
char* encontrarSuspeito(HashEntry *tabela[], const char *pista) {
    if (!pista || pista[0] == '\0') return NULL;
    unsigned long h = hash_djb2(pista) % HASH_SIZE;
    HashEntry *cur = tabela[h];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0) {
            return cur->suspeito;
        }
        cur = cur->next;
    }
    return NULL;
}

/* Exploração interativa da mansão — navega e coleta pistas automaticamente.
   Comentário: explorarSalas() — navega pela árvore e ativa o sistema de pistas. */
void explorarSalas(Sala *inicio, PistaNode **raizPistas) {
    if (!inicio) return;

    Sala *atual = inicio;
    char escolha;

    printf("Bem-vindo ao Detective Quest — Capítulo Final!\n");
    while (1) {
        printf("\nVocê está na: %s\n", atual->nome);

        if (atual->pista[0] != '\0' && !atual->pistaColetada) {
            printf("Pista encontrada: \"%s\"\n", atual->pista);
            *raizPistas = inserirPista(*raizPistas, atual->pista);
            atual->pistaColetada = 1;
            printf("Pista coletada e adicionada ao caderno de investigação.\n");
        } else if (atual->pista[0] != '\0' && atual->pistaColetada) {
            printf("Já coletou a pista desta sala.\n");
        } else {
            printf("Nenhuma pista nesta sala.\n");
        }

        printf("\nEscolha um caminho:\n");
        printf(" (e) Ir para a esquerda\n");
        printf(" (d) Ir para a direita\n");
        printf(" (s) Sair da exploração\n");
        printf("Sua escolha: ");
        if (scanf(" %c", &escolha) != 1) {
            limparBufferStdin();
            printf("Entrada inválida. Tente novamente.\n");
            continue;
        }

        if (escolha == 'e' || escolha == 'E') {
            if (atual->esquerda) atual = atual->esquerda;
            else printf("Não há sala à esquerda. Escolha outro caminho.\n");
        } else if (escolha == 'd' || escolha == 'D') {
            if (atual->direita) atual = atual->direita;
            else printf("Não há sala à direita. Escolha outro caminho.\n");
        } else if (escolha == 's' || escolha == 'S') {
            printf("Encerrando exploração...\n");
            return;
        } else {
            printf("Opção inválida. Use 'e', 'd' ou 's'.\n");
        }
    }
}

/* Percorre a BST e conta quantas pistas apontam para 'suspeitoAlvo'. Retorna o total.
   Implementado recursivamente. */
int contarPistasParaSuspeito(PistaNode *raiz, HashEntry *tabela[], const char *suspeitoAlvo) {
    if (!raiz) return 0;
    int count = 0;
    /* verificar esquerda */
    count += contarPistasParaSuspeito(raiz->esq, tabela, suspeitoAlvo);
    /* verificar este nó */
    char *sus = encontrarSuspeito(tabela, raiz->pista);
    if (sus != NULL) {
        if (strcmp(sus, suspeitoAlvo) == 0) count++;
    }
    /* direita */
    count += contarPistasParaSuspeito(raiz->dir, tabela, suspeitoAlvo);
    return count;
}

/* Fase final de acusação. Lista pistas coletadas, solicita acusação e decide.
   Comentário: verificarSuspeitoFinal() — conduz à fase de julgamento final. */
void verificarSuspeitoFinal(PistaNode *raizPistas, HashEntry *tabela[]) {
    char escolha[MAXNOME];
    printf("\n--- Pistas coletadas (ordem alfabética) ---\n");
    if (!raizPistas) {
        printf("Nenhuma pista coletada.\n");
    } else {
        exibirPistas(raizPistas);
    }

    printf("\nDigite o nome do suspeito que você deseja acusar (ex.: Sr. Black):\n");
    limparBufferStdin();
    if (!fgets(escolha, sizeof(escolha), stdin)) {
        printf("Erro na leitura. Abortando acusação.\n");
        return;
    }
    /* remover newline */
    escolha[strcspn(escolha, "\n")] = '\0';
    /* normalização simples: manter caso sensível conforme inserido na hash (hash armazena nomes tal quais) */
    if (strlen(escolha) == 0) {
        printf("Nenhum suspeito indicado. Fase de acusação cancelada.\n");
        return;
    }

    /* contar quantas pistas apontam para esse suspeito */
    int total = contarPistasParaSuspeito(raizPistas, tabela, escolha);
    printf("\nTotal de pistas que apontam para %s: %d\n", escolha, total);

    if (total >= 2) {
        printf("Resultado: Há evidências suficientes. %s é considerado(a) culpado(a)!\n", escolha);
    } else {
        printf("Resultado: Evidências insuficientes para condenar %s.\n", escolha);
    }
}

/* Libera memória alocada para salas */
void liberarSalas(Sala *raiz) {
    if (!raiz) return;
    liberarSalas(raiz->esquerda);
    liberarSalas(raiz->direita);
    free(raiz);
}

/* Libera toda a tabela hash */
void liberarHash(HashEntry *tabela[]) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashEntry *cur = tabela[i];
        while (cur) {
            HashEntry *tmp = cur;
            cur = cur->next;
            free(tmp);
        }
        tabela[i] = NULL;
    }
}

/* Limpa buffer stdin (útil antes de fgets após scanf) */
void limparBufferStdin(void) {
    int c;
    while ((c = getchar()) != EOF && c != '\n');
}

/* transforma string em minusculas (não usado para matching, só utilitário) */
void minusculas(char *s) {
    for (; *s; ++s) *s = (char) tolower((unsigned char) *s);
}

/* ------------------------
   main: monta a mansão, popula a hash e executa o jogo
   ------------------------*/
int main() {
    /* Montagem fixa da árvore (mesma mansão usada antes) */
    Sala *hall = criarSala("Hall de Entrada", "Envelope com um símbolo estranho");
    hall->esquerda = criarSala("Sala de Estar", "Pegadas de lama perto do tapete");
    hall->direita = criarSala("Biblioteca", "Página rasgada de diário");

    hall->esquerda->esquerda = criarSala("Cozinha", "Chave dourada enferrujada");
    hall->esquerda->direita  = criarSala("Sala de Jantar", "Guardanapo com anotação");

    hall->direita->esquerda  = criarSala("Quarto de Hóspedes", "Meia caída sob a cama");
    hall->direita->direita   = criarSala("Jardim Interno", "Pegadas que levam para o muro");

    /* Inicializa tabela hash (tudo NULL) */
    HashEntry *tabela[HASH_SIZE];
    for (int i = 0; i < HASH_SIZE; ++i) tabela[i] = NULL;

    /* Define suspeitos e associa pistas a suspeitos (estático) */
    /* Suspeitos: "Sr. Black", "Sra. White", "Dr. Green" (exemplos) */
    inserirNaHash(tabela, "Envelope com um símbolo estranho", "Sr. Black");
    inserirNaHash(tabela, "Pegadas de lama perto do tapete", "Dr. Green");
    inserirNaHash(tabela, "Página rasgada de diário", "Sra. White");
    inserirNaHash(tabela, "Chave dourada enferrujada", "Sr. Black");
    inserirNaHash(tabela, "Guardanapo com anotação", "Sra. White");
    inserirNaHash(tabela, "Meia caída sob a cama", "Dr. Green");
    inserirNaHash(tabela, "Pegadas que levam para o muro", "Sr. Black");
    /* Obs: é possível adicionar mais associações conforme desejar */

    /* raiz da BST de pistas coletadas (inicialmente vazia) */
    PistaNode *raizPistas = NULL;

    /* Exploração interativa com coleta automática de pistas */
    explorarSalas(hall, &raizPistas);

    /* Fase final: listar pistas e permitir acusação */
    verificarSuspeitoFinal(raizPistas, tabela);

    /* Liberar memória */
    liberarSalas(hall);
    liberarPistas(raizPistas);
    liberarHash(tabela);

    printf("\nObrigado por jogar Detective Quest — Capítulo Final!\n");
    return 0;
}

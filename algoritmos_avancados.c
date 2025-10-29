#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct sala {
    char nome[50];
    struct sala *esquerda;
    struct sala *direita;
} Sala;

/* Cria dinamicamente uma sala da mansão */
Sala* criarSala(const char nome[]) {
    Sala *nova = (Sala*) malloc(sizeof(Sala));
    strcpy(nova->nome, nome);
    nova->esquerda = NULL;
    nova->direita = NULL;
    return nova;
}

/* Função que permite o jogador explorar a mansão */
void explorarSalas(Sala *atual) {
    char escolha;

    while (atual != NULL) {
        printf("\nVocê está na: %s\n", atual->nome);

        // Se não houver caminhos para seguir, para a exploração
        if (atual->esquerda == NULL && atual->direita == NULL) {
            printf("Não há mais caminhos a seguir. Fim da exploração!\n");
            return;
        }

        printf("Escolha um caminho:\n");
        printf(" (e) Ir para a esquerda\n");
        printf(" (d) Ir para a direita\n");
        printf(" (s) Sair do jogo\n");
        printf("Sua escolha: ");
        scanf(" %c", &escolha);

        if (escolha == 'e')
            atual = atual->esquerda;
        else if (escolha == 'd')
            atual = atual->direita;
        else if (escolha == 's') {
            printf("Você saiu da exploração.\n");
            return;
        } else {
            printf("Opção inválida! Tente novamente.\n");
        }
    }
}

int main() {
    /* Montagem fixa da árvore (mapa da mansão) */
    Sala *hall = criarSala("Hall de Entrada");
    hall->esquerda = criarSala("Sala de Estar");
    hall->direita = criarSala("Biblioteca");

    hall->esquerda->esquerda = criarSala("Cozinha");
    hall->esquerda->direita = criarSala("Sala de Jantar");

    hall->direita->esquerda = criarSala("Quarto de Hóspedes");
    hall->direita->direita = criarSala("Jardim Interno");

    printf("Bem-vindo ao Detective Quest!\n");
    explorarSalas(hall);

    return 0;
}

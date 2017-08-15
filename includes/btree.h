/*  Gabriel Pinto de Camargo - 9293456
    Gabriel Simmel Nascimento - 9050232
    Marcos Cesar Ribeiro de Camargo - 9278045
    Victor Luiz Roquete Forbes - 9293394    */

#ifndef BTREE_H
#define BTREE_H

#include <stdio.h>

#define NIL -1 // Not In List.
#define PAGE_SIZE 64 // Página de disco de tamanho 64 bytes.

// Cálculo do M em função de PAGE_SIZE:
// sizeof(int) * (M - 1) + sizeof(int) * (M - 1) + sizeof(short) * M + sizeof(short) = PAGE_SIZE
// 2 * sizeof(int) * M - 2 * sizeof(int) + sizeof(short) * M + sizeof(short) = PAGE_SIZE
// 2 * sizeof(int) * M + sizeof(short) * M = PAGE_SIZE - sizeof(short) + 2 * sizeof(int)
// M * (2 * sizeof(int) + sizeof(short)) = PAGE_SIZE - sizeof(short) + 2 * sizeof(int)
// M = ((PAGE_SIZE - sizeof(short) + 2 * sizeof(int)) / (2 * sizeof(int) + sizeof(short)))

// SHORT == sizeof(short) e INT == sizeof(int)
#define SHORT 2
#define INT 4

// Garantindo que a ordem escolhida seja válida. A ordem M não pode ser escolhida tal que M % 3 == 2.
#if (((PAGE_SIZE - SHORT + 2 * INT) / (2 * INT + SHORT)) % 3 == 2)
    #define M (((PAGE_SIZE - sizeof(short) + 2 * sizeof(int)) / (2 * sizeof(int) + sizeof(short))) - 1)
#else
    #define M ((PAGE_SIZE - sizeof(short) + 2 * sizeof(int)) / (2 * sizeof(int) + sizeof(short)))
#endif

typedef struct BTree BTree;

/* Inicializa uma Árvore B* a partir de um arquivo de cabeçalho. */
BTree *btree_initialize(FILE *, FILE *);

/* Deleta uma Árvore B* e armazena o cabeçalho atualizado em disco. */
void btree_delete(BTree *);

/* Insere um novo par (chave, offset) na Árvore B* (no arquivo de índices). */
void btree_insert(BTree *, int, int);

/* Imprime uma representação da Árvore B* usando uma BFS. */
void print_tree(BTree *);

/* Busca na Árvore B* (no arquivo de índices) uma determinada chave e retorna
o seu offset associado. */
int btree_search(BTree *, int);

#endif
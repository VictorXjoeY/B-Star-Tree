/*  Gabriel Pinto de Camargo - 9293456
    Gabriel Simmel Nascimento - 9050232
    Marcos Cesar Ribeiro de Camargo - 9278045
    Victor Luiz Roquete Forbes - 9293394    */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "series.h"
#include "filemanip.h"
#include "btree.h"

/* Troca dois elementos de posição em um vetor. */
void swap(void *, int, int, int);

char *read_line(FILE *fp, char terminator){
    char *stretch = NULL;
    int i = 0;

    do{
        stretch = (char *)realloc(stretch, (i + 1) * sizeof(char));
        fscanf(fp, "%c", stretch + i);
        i++;
    }while (!feof(fp) && stretch[i - 1] != terminator);
    // Lendo até o final do arquivo ou até um caractere terminador
    // passado por parâmetro.

    // Adicionando um caractere '\0'.
    stretch[i - 1] = '\0';

    return stretch;
}

FILE *open_file(const char *filename, const char *directory, const char *extension){
    char *path = (char *)malloc((strlen(filename) + strlen(directory) + strlen(extension) + 3) * sizeof(char));
    FILE *fp;

    // Armazenando em uma string o "caminho" do arquivo em "path".
    sprintf(path, "%s/%s.%s", directory, filename, extension);

    // Tentando abrir o arquivo para escrita e leitura.
    fp = fopen(path, "r+");

    // Se o arquivo não existir, crie.
    if (fp == NULL){
        fp = fopen(path, "w+");
    }

    // Liberando a string.
    free(path);

    // Retornando o ponteiro do arquivo.
    return fp;
}

bool contains_character(const char *str, char c){
    // Enquanto o caractere não for '\0'.
	while (*str){
        // Se o caractere for o caractere buscado.
		if (*str == c){
			return true;
		}

        // Incrementando o endereço.
		str++;
	}

    // Não encontrou o caractere.
	return false;
}

void randomize_vector(void *v, int size, int n){
    int i, pos;

    // Trocando o elemento na posição i com o elemento em uma posição aleatória.
    for (i = 0; i < n; i++){
        pos = rand() % n;

        // Trocando de posição.
        swap(v, size, i, pos);
    }
}

bool id_already_used(BTree *bt, int id){
    return btree_search(bt, id) != NIL;
}

void swap(void *v, int size, int x, int y){
    void *aux;

    // Troca os elementos de posição apenas se a posição deles no vetor for diferente.
    if (x != y){
        // Alocando memória auxiliar.
        aux = malloc(size);

        // Trocando.
        memcpy(aux, v + x * size, size);
        memcpy(v + x * size, v + y * size, size);
        memcpy(v + y * size, aux, size);

        // Liberando memória auxiliar.
        free(aux);
    }
}
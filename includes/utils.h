/*	Gabriel Pinto de Camargo - 9293456
	Gabriel Simmel Nascimento - 9050232
	Marcos Cesar Ribeiro de Camargo - 9278045
	Victor Luiz Roquete Forbes - 9293394 	*/

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "btree.h"

#define and &&
#define or ||
#define not !

#define true 1
#define false 0

typedef unsigned char bool;

/* Lê caracteres de uma stream até um dado caractere terminador os retorna como string */
char *read_line(FILE *, char);

/* Abre um arquivo com o nome, o diretório e a extensão passados por parâmetro. */
FILE *open_file(const char *, const char *, const char *);

/* Verifica se uma string contém um determinado caractere. */
bool contains_character(const char *, char);

/* Aleatoriza as posições dos elementos de um vetor. */
void randomize_vector(void *, int, int);

/* Retorna true caso o ID buscado esteja presente no arquivo de dados. */ 
bool id_already_used(BTree *, int);

#endif
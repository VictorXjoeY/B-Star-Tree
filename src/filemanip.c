/*	Gabriel Pinto de Camargo - 9293456
	Gabriel Simmel Nascimento - 9050232
	Marcos Cesar Ribeiro de Camargo - 9278045
	Victor Luiz Roquete Forbes - 9293394 	*/

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "series.h"
#include "filemanip.h"

int insert(FILE *fp, const void *data, int size){
	int position;

	// Indo para o final do arquivo.
	fseek(fp, 0, SEEK_END);

	position = (int)ftell(fp);

	// Escrevendo os dados no arquivo.
	fwrite(data, size, 1, fp);

	// Colocando o delimitador.
	fprintf(fp, "%c", DELIMITADOR);

	return position;
}

void *retrieve_data(FILE *fp){
	void *data = malloc(NUMERO_DE_CAMPOS_INTEIROS * sizeof(int));
	int offset; // Utilizado tanto como offset, quanto para contar o tamanho atual de "data".

	// Lendo os 3 inteiros (id, year e season).
	fread(data, sizeof(int), NUMERO_DE_CAMPOS_INTEIROS, fp);
	offset = NUMERO_DE_CAMPOS_INTEIROS * sizeof(int);

	// Lendo caractere a caractere até o delimitador.
	do{
		// Alocando espaço para mais um caractere.
		data = realloc(data, (offset + sizeof(char)));

		// Lendo um caractere.
		fread(data + offset, sizeof(char), 1, fp);

		// Incrementando o offset.
		offset += sizeof(char);
	}while (*((char *)(data + offset - sizeof(char))) != DELIMITADOR);
	// Enquanto o último caractere lido não tiver sido o delimitador de registros.

	// Retornando os dados (concatenados).
	return data;
}
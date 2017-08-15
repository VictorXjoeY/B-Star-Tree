/*	Gabriel Pinto de Camargo - 9293456
	Gabriel Simmel Nascimento - 9050232
	Marcos Cesar Ribeiro de Camargo - 9278045
	Victor Luiz Roquete Forbes - 9293394 	*/

#ifndef FILE_MANIP_H
#define FILE_MANIP_H

typedef struct Header Header;

/* Insere um registro no final do arquivo e retorna seu offset. */
int insert(FILE *, const void *, int);

/* Recupera o registro na posição atual do arquivo.
O ponteiro do arquivo estará posicionado no primeiro byte
do próximo registro após essa função. */
void *retrieve_data(FILE *);

#endif
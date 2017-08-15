/*	Gabriel Pinto de Camargo - 9293456
	Gabriel Simmel Nascimento - 9050232
	Marcos Cesar Ribeiro de Camargo - 9278045
	Victor Luiz Roquete Forbes - 9293394 	*/

#ifndef SERIES_H
#define SERIES_H

// Número (fixo) de campos presentes no registro.
#define NUMERO_DE_CAMPOS_INTEIROS 3
#define NUMERO_DE_CAMPOS_STRINGS 4
	
// ID máximo e mínimo que pode ser gerado.
// #define MAX_ID 99999999
#define MAX_ID 99999
#define MIN_ID 0

// Definindo o delimitador de registros.
#define DELIMITADOR '#'

typedef struct Serie Serie;

/* Lê os dados de uma Série da stdin. */
Serie *read_serie();

/* Apaga a memória alocada para armazenar os dados de uma Série. */
void erase_serie(Serie *);

/* Concatena os dados de uma Série. */
void *get_data(const Serie *);

/* Desconcatena os dados de uma Série. */
Serie *get_serie(const void *);

/* Retorna o tamanho total (em bytes) dos dados de uma Serie. */
int get_size(const Serie *);

/* Retorna o id de uma Serie. */
int get_id(const Serie *);

/* Seta um id novo para uma série. */
void set_id(Serie *, int);

/* Imprime os dados de uma série. */
void print_serie(const Serie *);

/* Lê os dados de n séries contidas no arquivo "series.dat". */
Serie **read_generated_series(BTree *, const char *, int);

/* Apaga os dados das n séries lidas. */
void erase_generated_series(Serie **, int);

#endif
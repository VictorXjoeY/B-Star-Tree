/*	Gabriel Pinto de Camargo - 9293456
	Gabriel Simmel Nascimento - 9050232
	Marcos Cesar Ribeiro de Camargo - 9278045
	Victor Luiz Roquete Forbes - 9293394 	*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "utils.h"
#include "series.h"
#include "filemanip.h"
#include "btree.h"

// Defines para o menu do laço principal do programa.
#define GENERATE_SERIES 1
#define INSERT_SERIE 2
#define SEARCH_SERIE 3
#define PRINT_SERIES 4
#define PRINT_TREE 5

// Para que a ordem escolhida seja válida, o tamanho mínimo da página de disco deve ser 24 bytes.
#define MINIMUM_PAGE_SIZE 24

#define SHORT_INT_ERROR 1
#define PAGE_SIZE_TOO_SMALL_ERROR 2

/* Imprime os dados de todos os registros do arquivo de dados. */
void print_data(FILE *);

int main(int argc, char *argv[]){
	char *filename; // Nome do arquivo (sem a extensão).
	int offset, N, op, id, i; // Offset, opção do menu, ID de uma Série, iterador.
	void *data; // Dados.
	FILE *fp_data; // Ponteiro do arquivo de dados.
	FILE *fp_index; // Ponteiro do arquivo de índices.
	FILE *fp_header; // Ponteiro do arquivo de cabeçalho.
	Serie **gen; // Vetor de Séries geradas.
	Serie *s; // Série.
	BTree *bt; // Árvore B*.

	// Caso o sizeof(int) não seja 4 ou sizeof(short) não seja 2.
	if (SHORT != sizeof(short) or INT != sizeof(int)){
		printf("Os valores de sizeof(int) e/ou sizeof(short) nao correspondem aos valores definidos, por favor atualize-os no arquivo \"btree.h\".\n");
		return SHORT_INT_ERROR;
	}

	// Caso a ordem escolhida (em etapa de pré-compilação) seja menor do que o mínimo permitido.
	// A ordem deve ser pelo menos 2, pois para ordem M = 1 a quantidade de chaves em cada
	// nó será M - 1 = 1 - 1 = 0.
	if (M <= 1){
		printf("Tamanho da pagina de disco insuficiente. Favor inserir um valor maior ou igual a %d.\n", MINIMUM_PAGE_SIZE);
		return PAGE_SIZE_TOO_SMALL_ERROR;
	}

	// Setando uma seed de acordo com o horário do sistema.
	srand(time(NULL));

	// Lendo o nome do arquivo de dados.
	printf("Nome do arquivo de dados: ");
	filename = read_line(stdin, '\n');

	// Abrindo o arquivo de dados, o arquivo de índices e o arquivo de cabeçalho para leitura e escrita.
	fp_data = open_file(filename, "bin", "bin");
	fp_index = open_file(filename, "bin", "idx");
	fp_header = open_file(filename, "bin", "header");
	free(filename);

	// Inicializando a Árvore B*.
	bt = btree_initialize(fp_index, fp_header);

	// Caso a ordem do arquivo lido e a ordem do código sejam diferentes.
	if (bt == NULL){
		printf("Erro na criacao da arvore em cima do arquivo de indices.\n");
		// Fechando o arquivo de dados.
		fclose(fp_data);
		return 0;
	}

	// Setando a opção para 1 (para entrar no while).
	op = 1;

	// Loop principal.
	while (op){
		// Mensagens para o usuário relativas ao menu principal.
		printf("\n");
		printf("1 - Gerar series.\n");
		printf("2 - Inserir serie.\n");
		printf("3 - Buscar serie.\n");
		printf("4 - Imprimir series.\n");
		printf("5 - Imprimir Arvore B*.\n");
		printf("0 - Sair.\n");
		scanf("%d%*c", &op);
		printf("\n");

		switch (op){
			case GENERATE_SERIES:
				N = 0;

				// Recebendo do usuário quantas séries ele gostaria que fossem geradas automaticamente.
				while (N < 1 or N > 250){
					printf("Quantas series gostaria de gerar? (Maximo de 250): ");
					scanf("%d%*c", &N);
				}

				// Lê as Séries do arquivo "series.dat" e as retorna em um vetor gerado aleatoriamente.
				gen = read_generated_series(bt, "series.dat", N);
				
				// Inserindo as Séries no arquivo.
				for (i = 0; i < N; i++){
					// Concatenando todos os campos de cada registro.
					data = get_data(gen[i]);

					// Inserindo no arquivo de dados.
					offset = insert(fp_data, data, get_size(gen[i]));

					// Inserindo no arquivo de índices.
					btree_insert(bt, get_id(gen[i]), offset);

					// Desalocando memória alocada para armazenar a Série concatenada.
					free(data);
				}

				// Mensagem para o usuário.
				printf("Series geradas com sucesso.\n");

				// Liberando a memória alocada para armazenar as séries.
				erase_generated_series(gen, N);
				break;
			case INSERT_SERIE:
				// Lendo a série da stdin.
				s = read_serie();

				// Impedindo que o usuário digite um ID já presente no arquivo de dados.
				while (id_already_used(bt, get_id(s))){
					printf("\nID invalido, digite novamente: ");

					// Recebendo um novo ID.
					scanf("%d%*c", &id);
					set_id(s, id);
				}

				// Concatenando os dados da Série.
				data = get_data(s);

				// Inserindo os dados da Série no arquivo.
				offset = insert(fp_data, data, get_size(s));

				// Inserindo na Árvore B* (passando o ID e o offset).
				btree_insert(bt, get_id(s), offset);

				printf("\nSerie inserida no arquivo de dados com sucesso.\n");

				// Apagando a struct Série.
				erase_serie(s);

				// Desalocando memória alocada para armazenar a Série concatenada.
				free(data);
				break;
			case SEARCH_SERIE:
				// Recebendo o ID a ser buscado.
				printf("ID: ");
				scanf("%d%*c", &id);
				printf("\n");

				// Recuperando o offset na Árvore B*.
				offset = btree_search(bt, id);

				// Se a série não estiver no arquivo.
				if (offset == NIL){
					printf("Serie nao encontrada.\n");
				}
				else{
					// Recuperando a Série buscada.
					fseek(fp_data, offset, SEEK_SET);
					data = retrieve_data(fp_data);
					s = get_serie(data);

					// Imprimindo a Série.
					print_serie(s);
					printf("\n");

					// Liberando a memória auxiliar.
					free(data);
					erase_serie(s);
				}
				break;
			case PRINT_SERIES:
				// Imprimindo todos os registros presentes no arquivo.
				print_data(fp_data);
				break;
			case PRINT_TREE:
				// Imprimindo uma representação textual da Árvore B*.
				print_tree(bt);
				break;
			default:
				break;
		}
	}

	// Apagando a estrutura da Árvore B*.
	btree_delete(bt);

	// Fechando o arquivo de dados.
	fclose(fp_data);

	return 0;
}

void print_data(FILE *fp){
	void *data;
	Serie *s;

	// Posicionando o ponteiro no começo do arquivo.
	fseek(fp, 0, SEEK_SET);

	// Forçando a flag EOF.
	fscanf(fp, "%*c");

	printf("Pressione enter para ver a proxima serie.\n\n");

	while (!feof(fp)){
		// Voltando um byte atrás.
		fseek(fp, -sizeof(char), SEEK_CUR);

		// Recuperando o registro na posição atual.
		data = retrieve_data(fp);

		// Transformando os dados em uma struct Série.
		s = get_serie(data);

		// Ignorando um caractere da stdin (o enter a ser pressionado).
		scanf("%*c");

		// Imprimindo a Série lida.
		print_serie(s);
		printf("\n");

		// Liberando a memória alocada nessa iteração.
		erase_serie(s);
		free(data);

		// Forçando a flag EOF a ser setada.
		fscanf(fp, "%*c");
	}
}

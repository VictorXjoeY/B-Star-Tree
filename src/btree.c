/*  Gabriel Pinto de Camargo - 9293456
    Gabriel Simmel Nascimento - 9050232
    Marcos Cesar Ribeiro de Camargo - 9278045
    Victor Luiz Roquete Forbes - 9293394    */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "queue.h"
#include "btree.h"

#define OK NULL

typedef struct Header Header;
typedef struct Node Node;

// 30 bytes. Cabeçalho do arquivo de índices.
struct Header{
	int height; // Altura atual da Árvore.
	int order; // Ordem da Árvore.
	int minimum; // Ocupação mínima de um nó intermediário.
	int leaf_minimum; // Ocupação mínima de um nó folha.
	int maximum; // Ocupação máxima.
	int amount; // Quantidade de registros atualmente no arquivo.
	int page_size; // Tamanho da página de disco considerada para este arquivo.
	short root; // Ponteiro para a raiz.
};

// 64 bytes. Nó da árvore.
struct Node{
	int key[M - 1]; // 24 bytes - Chave primária (ID).
	int offset[M - 1]; // 24 bytes - Offset do registro no arquivo de dados.
	short children[M]; // 14 bytes - RRN dos nós filhos.
	short n; // 2 bytes - Número de chaves presentes no nó.
};

struct BTree{
	FILE *fp_index;
	FILE *fp_header;
	Header *header;
};

/* Inicializa um novo nó da Árvore. */
Node *node_initialize();

/* Retorna true se o nó for um nó folha. Retorna false caso contrário. */
bool node_is_leaf(Node *);

/* Retorna true se o nó estiver cheio. Retorna false caso contrário. */
bool node_is_full(Node *);

/* Função que imprime um nó da Árvore B*. */
void print_node(Node *);

/* Cria e inicializa um novo cabeçalho (para uma Árvore B* vazia). */
Header *header_initialize();

/* Lê o cabeçalho do arquivo de cabeçalho. */
Header *read_header(FILE *);

/* Retorna true se o arquivo de cabeçalho estiver vazio. Retorna false caso contrário. */
bool header_file_is_empty(FILE *);

/* Função recursiva para inserir na Árvore B* e consertar a Árvore na volta da recursão. */
Node *btree_insert_aux(BTree *, short, int *, int *, short *);

/* Função que realiza a operação de inserção simples em um nó folha. */
void simple_insertion(BTree *, Node *, int, int, int);

/* Função que realiza a operação de redistribuição. */
void redistribute(BTree *, Node *, Node *, Node *, short, short, short, int, int, int, short);

/* Função que realiza o split 2/3. */
void split(BTree *, Node *, Node *, Node *, int, int *, int *, short *);

/* Função que realiza o split da raíz. */
void split_root(BTree *, Node *, int, int, short);

/* Retorna a posição de inserção de uma dada chave. Retorna -1 caso a chave já esteja no nó. */
int get_insert_position(Node *, int);

/* Preenche o nó com os vetores a partir de uma dada posição. */
void fill_node_with_vectors(Node *, int, int *, int *, short *, int *);

/* Preenche os vetores com um dado nó a partir de uma dada posição. */
void fill_vectors_with_node(int *, int *, short *, Node *, int *);

/* Função que preenche os vetores auxiliares dado um nó e a chave a ser inserida. */
void fill_vectors_with_one_node(int *, int *, short *, Node *, int, int, short);

/* Fução auxiliar que preenche os vetores auxiliares dado o pai, os dois filhos e a chave a ser inserida. */
void fill_vectors_with_node_family(int *, int *, short *, Node *, Node *, Node *, int, int, int, short);

BTree *btree_initialize(FILE *fp_index, FILE *fp_header){
	BTree *bt = (BTree *)malloc(sizeof(BTree));

	// Criando o arquivo de cabeçalho caso ele não exista.
	if (header_file_is_empty(fp_header)){
		bt->header = header_initialize();
	}
	else{
		bt->header = read_header(fp_header);

		// Caso o arquivo lido possua um cabeçalho diferente do código.
		if (bt->header->page_size != PAGE_SIZE){
			printf("Arquivo de indices invalido!\n");
			fclose(fp_index);
			fclose(fp_header);
			free(bt->header);
			free(bt);
			return NULL;
		}

		printf("Arquivo carregado com sucesso!\n");
	}

	// Gravando ponteiro para o arquivo de índices e de cabeçalho.
	bt->fp_index = fp_index;
	bt->fp_header = fp_header;

	return bt;
}

void btree_delete(BTree *bt){
	fseek(bt->fp_header, 0, SEEK_SET);

	// Escrevendo o cabeçalho em disco.
	fwrite(bt->header, sizeof(Header), 1, bt->fp_header);

	// Liberando cabeçalho.
	free(bt->header);

	// Fechando arquivo de cabeçalho.
	fclose(bt->fp_header);

	// Fechando arquivo de índices.
	fclose(bt->fp_index);

	// Liberando estrutura da Árvore B.
	free(bt);
}

void btree_insert(BTree *bt, int key, int data_offset){
	short child = NIL; // Inicializando o RRN do filho à direita da nova chave.
	Node *root; // Nó auxiliar para armazenar a raíz.

	// Caso a Árvore esteja vazia.
	if (bt->header->root == NIL){
		// Inicializando um novo nó.
		root = node_initialize();

		// Inserindo o primeiro par (chave, offset).
		root->key[0] = key;
		root->offset[0] = data_offset;
		root->n++;

		// Atualizando o cabeçalho.
		bt->header->height++;
		bt->header->root = 0;

		// Escrevendo no arquivo de índices.
		fseek(bt->fp_index, 0, SEEK_SET);
		fwrite(root, PAGE_SIZE, 1, bt->fp_index);

		// Liberando memória auxiliar.
		free(root);
	}
	else{
		// Chamando função recursiva para inserir os elementos.
		// Passando o endereço da chave, do offset e do filho à direita da nova chave para
		// que a mesma região de memória seja acessível em todas as chamadas da recursão
		// (para que possamos propagar splits para cima).
		root = btree_insert_aux(bt, bt->header->root, &key, &data_offset, &child);

		// Deu overflow no nó raíz. Split 1/2.
		if (root){
			// Split 1/2.
			split_root(bt, root, key, data_offset, child);

			// Escrevendo a raíz de volta no disco.
			fseek(bt->fp_index, bt->header->root * PAGE_SIZE, SEEK_SET);
			fwrite(root, PAGE_SIZE, 1, bt->fp_index);

			// Liberando a raíz.
			free(root);

			// Sempre que há split na raíz a altura da Árvore é incrementada.
			bt->header->height++;
		}
	}

	// Incrementando contador de registros.
	bt->header->amount++;
}

int btree_search(BTree *bt, int key){
	int offset, x;
	Node *node;

	// Se a Árvore não estiver vazia.
	if (bt->header->root != NIL){
		offset = bt->header->root * PAGE_SIZE; // Offset inicial.
		node = (Node *)calloc(1, PAGE_SIZE); // Alocando espaço para um nó.

		// Buscando a chave iterativamente enquanto o nó atual não for folha.
		do{
			// Indo para o nó da árvore.
			fseek(bt->fp_index, offset, SEEK_SET);

			// Lendo o nó (a página de disco).
			fread(node, PAGE_SIZE, 1, bt->fp_index);

			// Procuro a posição dentro da página.
			for (x = 0; x < node->n and key > node->key[x]; x++);

			// Verificando se a chave foi encontrada.
			if (x == node->n or key != node->key[x]){
				// Procurando em um descendente caso não tenha encontrado.
				offset = node->children[x] * PAGE_SIZE;
			}
			else{
				// Achei. Retornando o offset.
				offset = node->offset[x];

				// Liberando memória auxiliar.
				free(node);

				// Retornando o offset no arquivo de dados.
				return offset;
			}
		}while (!node_is_leaf(node));

		// Liberando memória auxiliar.
		free(node);
	}

	// Registro não encontrado.
	return NIL;
}

void print_tree(BTree *bt){
	int i, offset, counter, level, left_amount, total_amount;
	Node *node;
	Queue *q; // Estrutura de dados auxiliar para executar uma BFS (Busca em Largura).

	printf("Ordem: %d / Tamanho da pagina de disco: %d / Quantidade de registros no arquivo: %d\n\n", bt->header->order, bt->header->page_size, bt->header->amount);

	// Caso a Árvore esteja vazia.
	if (bt->header->root == NIL){
		printf("Arvore vazia!\n");
		return;
	}

	// Inicializando uma fila para armazenar inteiros e um nó para ler do arquivo de índices.
	q = queue_new(sizeof(int));
	node = (Node *)malloc(PAGE_SIZE);

	// Calculando parâmetros de impressão.
	total_amount = M * 5 + (M - 1) * 5 + (M - 1) * 2;
	left_amount = (total_amount - strlen("Nivel X")) / 2;

	// Primeiro nó (raíz).
	offset = bt->header->root * PAGE_SIZE;

	// Inserindo o primeiro offset na fila.
	queue_push(q, &offset);

	// O contador indica quantos elementos tem no próximo nível.
	counter = queue_size(q);

	// Indicador do nível atual.
	level = 1;

	// Imprimindo o cabeçalho do primeiro nível da árvore.
	for (i = 0; i < left_amount; i++){
		printf("-");
	}

	printf("Nivel %d", level);

	for (i = left_amount + strlen("Nivel X"); i < total_amount; i++){
		printf("-");
	}

	printf("\n\n");

	// Enquanto a fila não estiver vazia.
	while (!queue_empty(q)){
		// Recuperando o próximo offset da fila.
		offset = *((int *)queue_front_ro(q));
		queue_pop(q);

		// Lendo o nó.
		fseek(bt->fp_index, offset, SEEK_SET);
		fread(node, PAGE_SIZE, 1, bt->fp_index);

		// Imprimindo as informações do nó.
		printf("RRN %03d:\n", offset / PAGE_SIZE);
		print_node(node);
		printf("\n");

		// Se o nó não for folha, insira seus filhos na fila.
		if (!node_is_leaf(node)){
			for (i = 0; i <= node->n; i++){
				offset = node->children[i] * PAGE_SIZE;
				queue_push(q, &offset);
			}
		}

		// Atualizando o contador.
		counter--;
		
		// Imprimindo o cabeçalho do próximo nível (caso haja próximo nível).
		if (counter == 0 and queue_size(q) != 0){
			// Atualizando o contador e o nível.
			counter = queue_size(q);
			level++;

			// Imprimindo rodapé do nível anterior.
			for (i = 0; i < total_amount; i++){
				printf("-");
			}

			printf("\n\n");

			// Imprimindo cabeçalho do próximo nível.
			for (i = 0; i < left_amount; i++){
				printf("-");
			}

			printf("Nivel %d", level);

			for (i = left_amount + strlen("Nivel X"); i < total_amount; i++){
				printf("-");
			}

			printf("\n\n");
		}
	}

	// Rodapé do último nível.
	for (i = 0; i < total_amount; i++){
		printf("-");
	}

	printf("\n");

	// Liberando memória.
	queue_delete(q);
	free(node);
}

/* -------------------- FUNÇÕES AUXILIARES ABAIXO -------------------- */

Node *node_initialize(){
	Node *node = (Node *)malloc(PAGE_SIZE);

	// Setando todos os seus valores para NIL.
	memset(node, NIL, PAGE_SIZE);

	// Inicializando a quantidade de chaves presentes no nó.
	node->n = 0;

	// Retornando o nó.
	return node;
}

bool node_is_leaf(Node *node){
	// O nó é folha caso não possua descendentes.
	return node->children[0] == NIL;
}

bool node_is_full(Node *node){
	// Verificando se o nó possui a quantidade máxima de elementos. 
	return node->n == M - 1;
}

void print_node(Node *node){
	int i;

	// Imprimindo o nó inteiro.
	for (i = 0; i < M - 1; i++){
		// Diferenciando caso não haja descendente.
		if (node->children[i] == NIL){
			printf("( * )");
		}
		else{
			printf("(%03d)", node->children[i]);
		}

		// Diferenciando caso esse elemento não esteja preenchido.
		if (node->key[i] == NIL){
			printf("  NIL  ");
		}
		else{
			printf(" %05d ", node->key[i]);
		}
	}

	// Diferenciando caso não haja descendente.
	if (node->children[i] == NIL){
		printf("( * )\n");
	}
	else{
		printf("(%03d)\n", node->children[i]);
	}
}

Header *header_initialize(){
	Header *h = (Header *)calloc(1, sizeof(Header));

	// Inicializando atributos.
	h->height = 0;
	h->order = M;
	h->leaf_minimum = (2 * M - 1) / 3;
	h->minimum = ((2 * M - 1) / 3) - 1;
	h->maximum = M - 1;
	h->root = NIL;
	h->amount = 0;
	h->page_size = PAGE_SIZE;

	return h;
}

Header *read_header(FILE *fp){
	Header *header = (Header *)malloc(sizeof(Header));

	// Lendo o cabeçalho da memória.
	fread(header, sizeof(Header), 1, fp);

	return header;
}

bool header_file_is_empty(FILE *fp){
	bool flag;

	// Indo para o final do arquivo.
	fseek(fp, 0, SEEK_END);

	// Verificando se o final do arquivo é o byte 0.
	flag = (ftell(fp) == 0);

	// Voltando para o começo do arquivo.
	fseek(fp, 0, SEEK_SET);

	// Retornando true se o arquivo estiver vazio.
	return flag;
}

Node *btree_insert_aux(BTree *bt, short current_rrn, int *key, int *data_offset, short *right_child){
	Node *current_node = (Node *)calloc(1, PAGE_SIZE); // Nó atual.
	Node *child = NULL; // Filho no qual foi inserido o novo elemento.
	Node *left_bro = NULL; // Irmão esquerdo do filho.
	Node *right_bro = NULL; // Irmão direito do filho.
	int offset, x;
	
	// Calculando o offset.
	offset = current_rrn * PAGE_SIZE;
	fseek(bt->fp_index, offset, SEEK_SET);
	fread(current_node, PAGE_SIZE, 1, bt->fp_index);

	// Se o nó atual não tiver descendentes.
	if (node_is_leaf(current_node)){ // Nó folha.
		// Overflow no nó folha.
		if (node_is_full(current_node)){
			return current_node;
		}

		// Inserção simples.
		simple_insertion(bt, current_node, current_rrn, *key, *data_offset);

		// Liberando o nó.
		free(current_node);

		return OK;
	}

	// Buscando a posição de inserção dentro do nó.
	x = get_insert_position(current_node, *key);

	if (x == -1){
		printf("Erro! Tentativa de inserir uma chave ja existente!\n");

		// Atualizando quantidade de registros.
		bt->header->amount--;

		// Liberando o nó.
		free(current_node);

		// Não haverá operação de inserão, portanto retorna OK.
		return OK;
	}
	
	// Obtendo o nó da chamada posterior.
	child = btree_insert_aux(bt, current_node->children[x], key, data_offset, right_child);

	// Se deu overflow no filho, child será diferente de NULL.
	if (child){
		// Se x não for a borda da esquerda.
		if (x - 1 >= 0){ // Tentando redistribuir para a esquerda.
			// Alocando espaço para armazenar o irmão da esquerda de child.
			left_bro = (Node *)calloc(1, PAGE_SIZE);

			// Lendo o irmão da esquerda.
			offset = current_node->children[x - 1] * PAGE_SIZE;
			fseek(bt->fp_index, offset, SEEK_SET);
			fread(left_bro, PAGE_SIZE, 1, bt->fp_index);

			// Se não estiver cheio, execute a redistribuição.
			if (!node_is_full(left_bro)){
				// Executando a redistribuição.
				// x - 1 é a posição do pai de child/left_bro
				redistribute(bt, current_node, left_bro, child, current_rrn, current_node->children[x - 1], current_node->children[x], x - 1, *key, *data_offset, *right_child);

				// Liberando os nós.
				free(current_node);
				free(left_bro);
				free(child);

				return OK;
			}
		}

		// Se x não for a borda da direita.
		if (x + 1 <= current_node->n){ // Tentando redistribuir para a direita.
			// Alocando espaço para armazenar o irmão da direita de child.
			right_bro = (Node *)calloc(1, PAGE_SIZE);
			
			// Lendo o irmão da direita.
			offset = current_node->children[x + 1] * PAGE_SIZE;
			fseek(bt->fp_index, offset, SEEK_SET);
			fread(right_bro, PAGE_SIZE, 1, bt->fp_index);

			// Se não estiver cheio, execute a redistribuição.
			if (!node_is_full(right_bro)){
				// Executando a redistribuição.
				// x é a posição do pai de child/right_bro
				redistribute(bt, current_node, child, right_bro, current_rrn, current_node->children[x], current_node->children[x + 1], x, *key, *data_offset, *right_child);

				// Liberando os nós.
				free(current_node);
				free(left_bro);
				free(right_bro);
				free(child);

				return OK;
			}
		}

		// Caso não tenha conseguido redistribuir acima, aplicar split 2/3.

		// Se houver irmão da esquerda, faço o split com o da esquerda.
		if (left_bro){
			// x - 1 é o pai de child/left_bro.
			split(bt, current_node, left_bro, child, x - 1, key, data_offset, right_child);

			// Liberando irmãos.
			free(left_bro);
			free(child);
			free(right_bro);

			// Se o split não gerou overflow no nó atual.
			if (*right_child == NIL){ // Split sem propagação.
				// Reescrevendo o nó atual no disco.
				offset = current_rrn * PAGE_SIZE;
				fseek(bt->fp_index, offset, SEEK_SET);
				fwrite(current_node, PAGE_SIZE, 1, bt->fp_index);

				free(current_node);

				return OK;
			}
			else{ // Split com propagação.
				return current_node;
			}
		}
		else{
			split(bt, current_node, child, right_bro, x, key, data_offset, right_child);

			// Liberando irmãos.
			free(left_bro);
			free(child);
			free(right_bro);

			// Se o split não gerou overflow no nó atual.
			if (*right_child == NIL){ // Split sem propagação.
				// Reescrevendo o nó atual no disco.
				offset = current_rrn * PAGE_SIZE;
				fseek(bt->fp_index, offset, SEEK_SET);
				fwrite(current_node, PAGE_SIZE, 1, bt->fp_index);

				free(current_node);

				return OK;
			}
			else{ // Split com propagação.
				return current_node;
			}
		}
	}

	// Liberando nó atual.
	free(current_node);

	// OK.
	return OK;
}

void simple_insertion(BTree *bt, Node *node, int rrn, int key, int data_offset){
	int i, offset;

	// Procurando posição de inserção.
	for (i = 0; i < node->n and key > node->key[i]; i++);

	// Se essa chave já foi inserida.
	if (key == node->key[i]){
		printf("Erro! Tentativa de inserir uma chave ja existente!\n");

		// Atualizando quantidade de registros.
		bt->header->amount--;
	}
	else{
		// Shiftando as chaves e os offsets (os filhos sempre são NIL nas inserções simples).
		memmove(node->key + i + 1, node->key + i, (M - 2 - i) * sizeof(int));
		memmove(node->offset + i + 1, node->offset + i, (M - 2 - i) * sizeof(int));

		// Inserindo no nó.
		node->key[i] = key;
		node->offset[i] = data_offset;
		node->n++;

		// Inserindo no disco.
		offset = rrn * PAGE_SIZE;
		fseek(bt->fp_index, offset, SEEK_SET);
		fwrite(node, PAGE_SIZE, 1, bt->fp_index);
	}
}

void redistribute(BTree *bt, Node *parent, Node *left, Node *right, short parent_rrn, short left_rrn, short right_rrn, int position, int key, int data_offset, short right_child){
	int x, left_amount, right_amount, total_amount, offset;
	int *key_vector, *offset_vector; // Vetores auxiliares.
	short *children_vector; // Vetor auxiliar.

	// Quantidade de chaves que haverá em cada filho.
	left_amount = (left->n + right->n + 1) / 2;
	right_amount = left->n + right->n + 1 - left_amount;

	// Quantidade total de chaves.
	total_amount = left->n + right->n + 2;

	// Alocando vetores auxiliares.
	key_vector = (int *)malloc(total_amount * sizeof(int));
	offset_vector = (int *)malloc(total_amount * sizeof(int));
	children_vector = (short *)malloc((total_amount + 1) * sizeof(short));

	// Preenchendo os vetores auxiliares para realizar a redistribuição.
	fill_vectors_with_node_family(key_vector, offset_vector, children_vector, parent, left, right, position, key, data_offset, right_child);

	// Inicializando os nós com -1.
	memset(left, NIL, PAGE_SIZE);
	memset(right, NIL, PAGE_SIZE);

	// Atualizando os tamanhos dos nós.
	left->n = left_amount;
	right->n = right_amount;

	x = 0;

	// Preenchendo o nó da esquerda.
	fill_node_with_vectors(left, left_amount, key_vector, offset_vector, children_vector, &x);

	// Preenchendo o pai.
	parent->key[position] = key_vector[x];
	parent->offset[position] = offset_vector[x];

	x++;

	// Preenchendo o nó da direita.
	fill_node_with_vectors(right, right_amount, key_vector, offset_vector, children_vector, &x);

	// Escrevendo o nó atual em disco (pai).
	offset = parent_rrn * PAGE_SIZE;
	fseek(bt->fp_index, offset, SEEK_SET);
	fwrite(parent, PAGE_SIZE, 1, bt->fp_index);

	// Inserindo o filho da esquerda em disco.
	offset = left_rrn * PAGE_SIZE;
	fseek(bt->fp_index, offset, SEEK_SET);
	fwrite(left, PAGE_SIZE, 1, bt->fp_index);

	// Inserindo o filho da direita em disco.
	offset = right_rrn * PAGE_SIZE;
	fseek(bt->fp_index, offset, SEEK_SET);
	fwrite(right, PAGE_SIZE, 1, bt->fp_index);

	// Liberando os vetores auxiliares.
	free(key_vector);
	free(offset_vector);
	free(children_vector);
}

void split(BTree *bt, Node *parent, Node *left, Node *right, int position, int *key, int *data_offset, short *right_child){
	int x, pos, left_amount, middle_amount, right_amount, total_amount, offset;
	int *key_vector, *offset_vector; // Vetores auxiliares.
	short *children_vector; // Vetor auxiliar.
	Node *middle;
	short rrn;

	// Quantidade de chaves que haverá em cada filho.
	left_amount = (2 * (M - 1) + 1) / 3;
	middle_amount = (2 * (M - 1) + 1) / 3;
	right_amount = 2 * (M - 1) - left_amount - middle_amount;

	// Quantidade total de chaves.
	total_amount = left_amount + right_amount + middle_amount + 2;

	// Alocando vetores auxiliares.
	key_vector = (int *)malloc(total_amount * sizeof(int));
	offset_vector = (int *)malloc(total_amount * sizeof(int));
	children_vector = (short *)malloc((total_amount + 1) * sizeof(short));

	// Preenchendo os vetores auxiliares para realizar a redistribuição.
	fill_vectors_with_node_family(key_vector, offset_vector, children_vector, parent, left, right, position, *key, *data_offset, *right_child);

	// Inicializando os nós com -1.
	memset(left, NIL, PAGE_SIZE);
	middle = node_initialize();
	memset(right, NIL, PAGE_SIZE);

	// Atualizando os tamanhos dos nós.
	left->n = left_amount;
	middle->n = middle_amount;
	right->n = right_amount;

	x = 0;

	// Preenchendo o nó da esquerda.
	fill_node_with_vectors(left, left_amount, key_vector, offset_vector, children_vector, &x);

	// Preenchendo o primeiro pai.
	parent->key[position] = key_vector[x];
	parent->offset[position] = offset_vector[x];
	x++;

	// Preenchendo o nó do meio.
	fill_node_with_vectors(middle, middle_amount, key_vector, offset_vector, children_vector, &x);

	// Gravando a posição que pertencerá ao (segundo) pai (pai promovido).
	pos = x;
	x++;

	// Preenchendo o nó da direita.
	fill_node_with_vectors(right, right_amount, key_vector, offset_vector, children_vector, &x);

	// Inserindo em disco o nó que foi considerado que surgiu do split (right).
	fseek(bt->fp_index, 0, SEEK_END);
	rrn = ftell(bt->fp_index) / PAGE_SIZE;
	fwrite(right, PAGE_SIZE, 1, bt->fp_index);

	// Inserindo em disco o nó da esquerda atualizado.
	offset = parent->children[position] * PAGE_SIZE;
	fseek(bt->fp_index, offset, SEEK_SET);
	fwrite(left, PAGE_SIZE, 1, bt->fp_index);

	// Inserindo em disco o nó do meio atualizado.
	offset = parent->children[position + 1] * PAGE_SIZE;
	fseek(bt->fp_index, offset, SEEK_SET);
	fwrite(middle, PAGE_SIZE, 1, bt->fp_index);

	// Liberando a memória alocada para o novo nó.
	free(middle);

	// Se o nó pai de cima estiver cheio.
	if (node_is_full(parent)){
		*key = key_vector[pos];
		*data_offset = offset_vector[pos];
		*right_child = rrn;
	}
	else{
		position++;

		// Promovendo.
		// Shiftando para abrir espaço para o novo pai.
		memmove(parent->key + position + 1, parent->key + position, (M - 2 - position) * sizeof(int));
		memmove(parent->offset + position + 1, parent->offset + position, (M - 2 - position) * sizeof(int));
		memmove(parent->children + position + 2, parent->children + position + 1, (M - 2 - position) * sizeof(short));

		// Inserindo o novo pai, com o filho da direita associado sendo o novo nó.
		parent->key[position] = key_vector[pos];
		parent->offset[position] = offset_vector[pos];
		parent->children[position + 1] = rrn;
		*right_child = NIL;

		// Uma chave foi promovida.
		parent->n++;
	}

	// Liberando vetores auxiliares.
	free(key_vector);
	free(offset_vector);
	free(children_vector);
}

void split_root(BTree *bt, Node *root, int key, int data_offset, short right_child){
	int x, left_amount, right_amount, total_amount; // Indexadores e tamanho dos novos nós.
	int *key_vector, *offset_vector; // Vetores auxiliares.
	short *children_vector; // Vetor auxiliar.
	Node *left, *right; // Nó 
	short rrn;

	// Quantidade de chaves que haverá em cada filho.
	left_amount = (M - 1) / 2;
	right_amount = M - 1 - left_amount;

	// Quantidade total de chaves.
	total_amount = left_amount + right_amount + 1;

	// Alocando vetores auxiliares.
	key_vector = (int *)malloc(total_amount * sizeof(int));
	offset_vector = (int *)malloc(total_amount * sizeof(int));
	children_vector = (short *)malloc((total_amount + 1) * sizeof(short));

	// Preenchendo vetores auxiliares.
	fill_vectors_with_one_node(key_vector, offset_vector, children_vector, root, key, data_offset, right_child);

	// Inicializando os nós com -1.
	left = node_initialize();
	memset(root, NIL, PAGE_SIZE);
	right = node_initialize();

	// Atualizando os tamanhos dos nós.
	left->n = left_amount;
	right->n = right_amount;
	root->n = 1;

	x = 0;

	// Preenchendo o nó da esquerda.
	fill_node_with_vectors(left, left_amount, key_vector, offset_vector, children_vector, &x);

	// Preenchendo o pai.
	root->key[0] = key_vector[x];
	root->offset[0] = offset_vector[x];
	
	x++;

	// Preenchendo o nó da direita.
	fill_node_with_vectors(right, right_amount, key_vector, offset_vector, children_vector, &x);

	// Inserindo o nó da esquerda que surgiu do split.
	fseek(bt->fp_index, 0, SEEK_END);
	rrn = ftell(bt->fp_index) / PAGE_SIZE;
	fwrite(left, PAGE_SIZE, 1, bt->fp_index);

	// Atualizando filho da esquerda da "nova" raíz.
	root->children[0] = rrn;

	// Inserindo o nó da direita que surgiu do split.
	fseek(bt->fp_index, 0, SEEK_END);
	rrn = ftell(bt->fp_index) / PAGE_SIZE;
	fwrite(right, PAGE_SIZE, 1, bt->fp_index);

	// Atualizando filho da esquerda da "nova" raíz.
	root->children[1] = rrn;

	// Liberando a memória alocada para os novos nós.
	free(left);
	free(right);

	// Liberando vetores auxiliares.
	free(key_vector);
	free(offset_vector);
	free(children_vector);
}

int get_insert_position(Node *node, int key){
	int i;

	// Procuro a posição dentro da página. (Podemos trocar para uma busca binária).
	for (i = 0; i < node->n and key > node->key[i]; i++);

	// Se essa chave já foi inserida antes.
	if (i != node->n and key == node->key[i]){
		return -1;
	}

	return i;
}

void fill_node_with_vectors(Node *node, int length, int *key_vector, int *offset_vector, short *children_vector, int *x){
	int i = 0;

	node->children[i] = children_vector[*x];

	for (; i < length; i++, (*x)++){
		node->key[i] = key_vector[*x];
		node->offset[i] = offset_vector[*x];
		node->children[i + 1] = children_vector[(*x) + 1];
	}
}

void fill_vectors_with_node(int *key_vector, int *offset_vector, short *children_vector, Node *node, int *x){
	int i = 0;

	// Primeiro filho.
	children_vector[*x] = node->children[i];

	// Resto do nó.
	for (; i < node->n; i++, (*x)++){
		key_vector[*x] = node->key[i];
		offset_vector[*x] = node->offset[i];
		children_vector[(*x) + 1] = node->children[i + 1];
	}
}

void fill_vectors_with_one_node(int *key_vector, int *offset_vector, short *children_vector, Node *node, int key, int data_offset, short right_child){
	int i, x;

	// Indexa os vetores.
	x = 0;

	// Passando o nó para os vetores auxiliares.
	fill_vectors_with_node(key_vector, offset_vector, children_vector, node, &x);

	// Procurando posição de inserção da nova chave nos vetores auxiliares.
	for (i = 0; i < node->n and key > key_vector[i]; i++);

	// Shiftando todos os elementos um espaço para a direita.
	memmove(key_vector + i + 1, key_vector + i, (node->n - i) * sizeof(int));
	memmove(offset_vector + i + 1, offset_vector + i, (node->n - i) * sizeof(int));
	memmove(children_vector + i + 2, children_vector + i + 1, (node->n - i) * sizeof(short));

	// Inserindo nos vetores auxiliares.
	key_vector[i] = key;
	offset_vector[i] = data_offset;
	children_vector[i + 1] = right_child;
}

void fill_vectors_with_node_family(int *key_vector, int *offset_vector, short *children_vector, Node *parent, Node *left, Node *right, int position, int key, int data_offset, short right_child){
	int total, x, i;

	// Total de chaves e indexador dos vetores.
	total = left->n + right->n + 1;
	x = 0;

	// Passando o nó da esquerda.
	fill_vectors_with_node(key_vector, offset_vector, children_vector, left, &x);

	// Passando o pai.
	key_vector[x] = parent->key[position];
	offset_vector[x] = parent->offset[position];

	x++;

	// Passando o nó da direita.
	fill_vectors_with_node(key_vector, offset_vector, children_vector, right, &x);

	// Procurando posição de inserção da nova chave nos vetores auxiliares.
	for (i = 0; i < total and key > key_vector[i]; i++);

	// Shiftando todos os elementos um espaço para a direita.
	memmove(key_vector + i + 1, key_vector + i, (total - i) * sizeof(int));
	memmove(offset_vector + i + 1, offset_vector + i, (total - i) * sizeof(int));
	memmove(children_vector + i + 2, children_vector + i + 1, (total - i) * sizeof(short));

	// Inserindo nos vetores auxiliares.
	key_vector[i] = key;
	offset_vector[i] = data_offset;
	children_vector[i + 1] = right_child;
}
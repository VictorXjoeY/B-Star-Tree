/* Victor Forbes - 9293394 */

#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "queue.h"

typedef struct Node Node;

struct Node{
	void *element;
	Node *next;
	Node *previous;
};

struct Queue{
	Node *front;
	Node *back;
	int size;
	int element_size;
};

/* Creates a node. */
Node *queue_node_new(const void *, int);

/* Erases a node from the Queue. */
void queue_node_delete(Node *);

Queue *queue_new(int element_size){
	Queue *q = (Queue *)malloc(sizeof(Queue));

	q->element_size = element_size;
	q->front = NULL;
	q->back = NULL;
	q->size = 0;

	return q;
}

void queue_clear(Queue *q){
	while (q->size){
		queue_pop(q);
	}
}

void queue_delete(Queue *q){
	queue_clear(q);
	free(q);
}

void queue_push(Queue *q, const void *element){
	if (q->size){
		q->back->next = queue_node_new(element, q->element_size);
		q->back->next->previous = q->back;
		q->back = q->back->next;
	}
	else{
		q->back = q->front = queue_node_new(element, q->element_size);
	}

	q->size++;
}

void *queue_front(const Queue *q){
	void *element = NULL;

	if (q->size){
		element = malloc(q->element_size);
		memcpy(element, q->front->element, q->element_size);
	}

	return element;
}

const void *queue_front_ro(const Queue *q){
	return q->size ? q->front->element : NULL;
}

void queue_pop(Queue *q){
	if (q->size){
		if (q->front->next){
			q->front = q->front->next;
			queue_node_delete(q->front->previous);
		}
		else{
			queue_node_delete(q->front);
			q->front = NULL;
			q->back = NULL;
		}

		q->size--;
	}
}

bool queue_empty(const Queue *q){
	return !q->size;
}

int queue_size(const Queue *q){
	return q->size;
}

Node *queue_node_new(const void *element, int element_size){
	Node *n = (Node *)malloc(sizeof(Node));

	n->element = malloc(element_size);
	memcpy(n->element, element, element_size);

	n->next = NULL;
	n->previous = NULL;

	return n;
}

void queue_node_delete(Node *n){
	free(n->element);
	free(n);
}
/* Victor Forbes - 9293394 */

#ifndef QUEUE_H
#define QUEUE_H

#include "utils.h"

typedef struct Queue Queue;

/* O(1) - Creates a Queue for elements of size "element_size". */
Queue *queue_new(int);

/* O(1) - Erases every element in the Queue. */
void queue_clear(Queue *);

/* O(1) - Destroys Queue. */
void queue_delete(Queue *);

/* O(1) - Inserts an element at the back of the Queue. */
void queue_push(Queue *, const void *);

/* O(1) - Returns a copy of the element that is on the front of the Queue. */
void *queue_front(const Queue *);

/* O(1) - Returns a reference to the element that is on the front of the Queue. */
const void *queue_front_ro(const Queue *);

/* O(1) - Erases the back element from the Queue. */
void queue_pop(Queue *);

/* O(1) - Returns true if the Queue is empty and false otherwise. */
bool queue_empty(const Queue *);

/* O(1) - Returns the amount of elements currently in the Queue. */
int queue_size(const Queue *);

#endif
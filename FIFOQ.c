/*
 *	FIFOQ.c
 *  Author: Marco Cirillo
 *
 * 	A First-In First-Out (FIFO) Queue 
 */

#ifndef STDLIB_H
#define STDLIB_H
#include <stdlib.h>
#endif /* STDLIB_H */

#include "FIFOQ.h"

/* Initialiazation */

/** Initializes the FIFO Q */
void FIFOQ_init (FIFOQ *q) {
	q->head = q->tail = NULL;
    q->size = 0;
}
 
/* Push Functions */

/** Pushes back a node in the queue */
void q_push_back (FIFOQ *q, Node *n) {
	if (q->size <= 0) {
		q->head = q->tail = n;
		n->next = q->tail;
		q->size = 1;
	} else {
		// Point old tail to new tail
		Node *oldTail = q->tail;
		oldTail->next = n;
		q->tail = n;
		
		q->size++;			// increment size
	}
}

/** Pushes back a char in the queue */
void q_push_back_data (FIFOQ *q, char data, off_t offset) {

	// Create new Node
	Node *n = malloc(sizeof(n));
	n->data = data;
	n->offset = offset;
	
	q_push_back(q, n);	// push using our common q_push_back function
}

/* Pop Functions */

/** Pops the top Node. Returns NULL if queue is empty */
Node * q_pop (FIFOQ *q) {

	if (q->size <= 0) return NULL;	// return null if empty queue

	Node * result = q->head;		// remove head Node
	q->head = q->head->next;		// set new head to the next in line
	q->size--;						// decrement size
	return result;					// return old head Node
}

/** Returns the char data from the top Node. Returns NULL if queue is empty */
char q_pop_char (FIFOQ *q) {
	
	if (q->size <= 0) return (char) NULL;	// return null if empty queue
	
	Node * oldNode = q_pop(q);		// remove head Node
	char result = oldNode->data;	// store data from head Node
	free(oldNode);					// oldNode can be safely freed
	return result;					// return the data from the old head Node
}

/** Peeks the data from the top Node. Returns NULL if queue is empty */
char q_peek (FIFOQ *q) {
	if (q->size <= 0) return (char) NULL;	// return null if empty queue
	else return q->head->data;		// data from the head of the queue
}

void test () {
	/* Testing Functions */
	printf("Testing FIFOQ\n");
	
	// Create and initialize the queue
	FIFOQ q;
	FIFOQ_init(&q);
	
	printf("Queue Initialization: %s\n", 
		(q.size == 0) && (q.head == NULL) && (q.head == q.tail) ? "PASS" : "FAIL");
		
	// Push 11 characters
	char charArray[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k' };
	int i;
	for (i = 0; i < 11; i++) {
		q_push_back_data(&q, charArray[i], 1);
	}
	
	printf("Queue Push Back: %s\n", 
		(q.size == 11) &&  (q.head != q.tail) ? "PASS" : "FAIL");
	
	// Peek top Node's data
	printf("Queue Peek: %s\n", (q.head->data == 'a') ? "PASS" : "FAIL");
	
	// Pop all nodes
	int valid;
	for (i = 0; i < 11; i++) {
		valid = q_pop_char(&q) == charArray[i];
	}
	printf("Queue Pop: %s\n", valid ? "PASS" : "FAIL");
}

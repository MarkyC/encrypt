/*
 *	FIFOQ.h
 *  Author: Marco Cirillo
 *
 * 	A First-In First-Out (FIFO) Queue 
 */
 
#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif /* STDIO_H */

/* Structs */

typedef struct Node Node;
typedef struct FIFOQ FIFOQ;

struct Node {
	char data;				// the data for this Node
	off_t offset;			// offset of data byte in the file
	Node *next;				// next Node in the queue
};
 
struct FIFOQ {
	Node *head;				// first Node in the queue
	Node *tail;				// last Node in the queue
	int size;				// number of Nodes
};

/* Initialiazation */

/** Initializes the FIFO Q */
void FIFOQ_init (FIFOQ *q);

/* Push Functions */

/** Pushes back a node in the queue */
void q_push_back (FIFOQ *q, Node *n);

/** Pushes back a char in the queue */
void q_push_back_data (FIFOQ *q, char data, off_t offset);

/* Pop Functions */

/** Pops the top Node. Returns NULL if queue is empty */
Node * q_pop (FIFOQ *q);

/** Returns the char data from the top Node. Returns NULL if queue is empty */
char q_pop_char (FIFOQ *q);

/** Peeks the data from the top Node. Returns NULL if queue is empty */
char q_peek (FIFOQ *q);

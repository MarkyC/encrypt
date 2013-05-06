/**
  *
  *	encrypt.h
  *
  *	Implements a simple file encryption system using pthreads to encrypt and decrypt the data.
  *
  * Uses the following formula to encryption:
  * if(data>31 && data<127 )
  *     data = (((int)data-32)+2*95+KEY)%95+32;
  *
  * Uses the following formula for decryption:
  * if (data>31 && data<127 )
  *     data = (((int)data-32)+2*95-KEY)%95+32 ;
  *
  *
  *		Family Name:	Cirillo
  *		 Given Name:	Marco
  *	 Student Number:	210272037
  *		  CSE Login:	CSE 93167
  *
  *
  */
  
#ifndef STDLIB_H
#define STDLIB_H
	#include <stdlib.h>
#endif /* STDLIB_H */

#ifndef CTYPE_H
#define CTYPE_H
#include <ctype.h>
#endif /* CTYPE_H */

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif /* STDIO_H */

#include <assert.h>
#include <semaphore.h>
#include <string.h>
#include <pthread.h>
#include "FIFOQ.c"

#define TEN_MILLIS_IN_NANOS 10000000 
#define KEY_MIN -127
#define KEY_MAX 127


/* Helper Functions **/

/** Checks Arguments for saneness. To be run before program does any work */
int check_args(char **args);

/** Prints Usage Information **/
void print_usage(char * filename);

/** Initializes Global Variables to be used by this program **/
void init_globals();

/** Waits for between 0 and 0.01s **/
void randomWait();

/** Basic Encryption Function **/
char encrypt(char data);

/** Pushes a new Node into the queue atomically using a mutex **/
void atomic_q_push_back(FIFOQ *q, Node *n);

/** Pops the head Node off of the queue atomically using a mutex **/
Node * atomic_q_pop(FIFOQ *q);

/** Reads one byte from the file, storing it in a node **/
void read_file(Node *result);

/** Writes data and offset to file_out **/
void write_file(char data, off_t offset);

/** Writes a Node to file_out **/
void write_node(Node *n);


/* Worker Thread Functions */

/** Reads characters from file_in into the queue **/
void *read_in();

/** Removes Nodes from the unprocessed queue, encrypts them, 
  *	then pushes them into the processed queue 
 **/
void *encrypt_data();

/** Removes Nodes from the processed queue and writes to file_out **/
void *write_out();


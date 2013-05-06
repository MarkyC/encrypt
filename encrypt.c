/**
  *
  *	encrypt.c
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
  
#include "encrypt.h"

/* Global Variables **/

int key;				// The key value used for encryption and decryption
int nIn;				// The number of in threads
int nWork;				// The number of worker threads
int nOut;				// The number of out threads

int file_read;			// If the file has been completely read
int file_processed;		// If the file has been completely encrypted

FILE *file_in;			// The input file
FILE *file_out;			// The output file

FIFOQ unprocessedQ;		// unencrypted characters waiting for encryption
FIFOQ processedQ;		// encrypted characters waiting to be written

pthread_t *inThread;	// Holds all IN threads
pthread_t *workThread;	// Holds all WORK threads
pthread_t *outThread;	// Holds all OUT threads

pthread_mutex_t mutex;	// The mutex used to block processes

/* Helper Functions **/

/** Checks Arguments for saneness. To be run before program does any work **/
int check_args(char **args) {
	
	// grab all integer arguments
	key = atoi(args[1]);
	nIn = atoi(args[2]);
	nWork = atoi(args[3]);
	nOut = atoi(args[4]);
	
	// if file_in and file_out are the same file, fopen() will destroy it, 
	// and this program will not work
	if (!strcmp(args[5], args[6])) {
		printf("Error: file_in and file_out are the same\n\n"); 
		return 0;
	}
	
	// grab all file pointer arguments
	file_in = fopen(args[5], "r");
	file_out = fopen(args[6], "w");

	// check key for proper range
	if ((key < KEY_MIN) || (key > KEY_MAX)) {printf("Error: Invalid Key\n\n"); return 0;}	 
	
	// check integer arguments are greater than 1
	if ((nIn < 1) || (nWork < 1) || (nOut < 1)) {
		printf("Error: nIn, nWork, and nOut should all be >= 1\n\n");
		return 0;
	}
	
	// check if files were successfully opened
	if ((!file_in) || (!file_out)) {printf("Error: Unable to open file\n\n"); return 0;}
	
	return 1;	// everything seems to be okay
}

/** Prints Usage Information **/
void print_usage(char * filename) {
	// Using the given usage information
	printf("Usage: %s <KEY> <nIN> <nWORK> <nOUT> <file_in> <file_out>\n", filename);
	printf(
	"<KEY>: the key value used for encryption or decryption, and its valid value is "
	"from %d to %d. If it is positive, WORK threads use  <KEY> as the KEY value "
	"to encrypt each data byte. If it is negative, WORK threads use the absolute "
	"value of <KEY> as the key value to decrypt each data byte.\n", KEY_MIN, KEY_MAX);
	printf("<nIN>: the number of IN threads to create. There should be at least 1.\n");
	printf("<nWORK>: the number of WORK threads to create. There should be at least 1.\n");
	printf("<nOUT>: the number of OUT threads to create. There should be at least 1.\n");
	printf(
	"<file_in>: the pathname of the file to be converted. It should exist and be "
	"readable.\n");
	printf("<file_out>: the name to be given to the target file. If a file with that name "
	"already exists, it should be overwritten.\n");
}

/** Initializes Global Variables to be used by this program **/
void init_globals() {
	file_read = file_processed = 0;

	// Initialize queues
	FIFOQ_init(&unprocessedQ);
	FIFOQ_init(&processedQ);
	
	// initialize in, out, work threads
	inThread	= malloc(nIn 	* sizeof(*inThread));
	workThread	= malloc(nWork 	* sizeof(*workThread));
	outThread	= malloc(nOut	* sizeof(*outThread));
	
	pthread_mutex_init(&mutex, NULL);	// initialize the mutex
}

/** Waits for between 0 and 0.01s **/
void randomWait() {
	struct timespec t;

	t.tv_sec = 0;
	t.tv_nsec = rand()%(TEN_MILLIS_IN_NANOS+1);

	nanosleep(&t, NULL);
}

/** Basic Encryption Function **/
char encrypt(char data) {

	if ((int) data>31 && (int) data<127) {
		data = (((int)data-32)+2*95+key)%95+32;
	}
	
	return data;
}

/** Reads one byte from the file, storing it in a node **/
void read_file(Node *result) {
	pthread_mutex_lock(&mutex);			// wait for mutex
	// Begin Critical Section: Reading From File

	result->offset = ftell(file_in);    // get position of byte in file 
	result->data = fgetc(file_in);      // read byte from file 

	
	// End Critical Section: Reading From File
	pthread_mutex_unlock(&mutex);		// release mutex lock
	
}

/** Pushes a new Node into the queue atomically using a mutex **/
void atomic_q_push_back(FIFOQ *q, Node *n) {
	pthread_mutex_lock(&mutex);			// wait for mutex
	// Begin Critical Section: Pushing To Queue
	
	q_push_back(q, n);
	
	// End Critical Section: Pushing To Queue
	pthread_mutex_unlock(&mutex);		// release mutex lock
}

/** Pops the head Node off of the queue atomically using a mutex **/
Node * atomic_q_pop(FIFOQ *q) {
	pthread_mutex_lock(&mutex);			// wait for mutex
	// Begin Critical Section: Popping From Queue
	
	Node *result = q_pop(q);
	
	// End Critical Section: Popping From Queue
	pthread_mutex_unlock(&mutex);		// release mutex lock
	
	return result;
}

/** Writes data and offset to file_out **/
void write_file(char data, off_t offset) {

	pthread_mutex_lock(&mutex);		// wait for mutex
	// Begin Critical Section: Write To File
	
	if (fseek(file_out, offset, SEEK_SET) == -1) {
		// attempt to seek to the desired offset
		printf("error setting output file position to %u\n", (unsigned int) offset);
		exit(-1);
	}
	if (fputc(data, file_out) == EOF) {			
		// attempt to write the desired data
		printf("error writing byte %d to output file\n", data);
		exit(-1);
	}

	// End Critical Section: Write To File
	pthread_mutex_unlock(&mutex);	// release mutex lock
}

/** Writes a Node to file_out **/
void write_node(Node *n) {
	char data = n->data;
	off_t offset = n->offset;
	
	write_file(data, offset);
	
	free(n);
}


/* Worker Thread Functions */

/** Reads characters from file_in into the queue **/
void *read_in() {
	printf("Beginning Read\n");
	while(!feof(file_in)) {
		printf("file processed: %d\n", file_processed);
		
		randomWait();								// threads must wait before doing work
		
		Node *n = malloc(sizeof(n));				// The new Node to add to the queue
		read_file(n);								// First Critical Section: Read char from file
		
		if (n->data != EOF) 
			atomic_q_push_back(&unprocessedQ, n);	// Second Critical Section: Write into queue 
	}
	
	pthread_exit(NULL);
}

/** Removes Nodes from the unprocessed queue, encrypts them, 
  *	then pushes them into the processed queue
 **/
void *encrypt_data() {
	printf("file processed: %d\n", file_processed);
	while (!file_read) {
		randomWait();							// threads must wait before doing work
		
		if (unprocessedQ.size == 0) continue;	// wait til buffer has items
		
		Node *n = atomic_q_pop(&unprocessedQ);	// First Critical Sect: remove head Node from queue
		
		n->data = encrypt(n->data);				// encrypt Node's data
		
		atomic_q_push_back(&processedQ, n);		// Second Critical Section: push to processed queue
	}
	
	pthread_exit(NULL);
}

/** Removes Nodes from the processed queue and writes to file_out **/
void *write_out() {
	printf("file processed: %d\n", file_processed);
	while (!file_processed) {
		randomWait();							// threads must wait before doing work
		
		if (processedQ.size == 0) continue;		// wait til queue has items
		
		Node *n = atomic_q_pop(&processedQ);		// First Critical Section: Pop From Queue
		
		write_node(n);							// Second Critical Section: Write to file_out

	}
	
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

	// Check and Initialize args
	if ((argc != 7) && (!check_args(argv))) {
		print_usage(argv[0]); 
		return EXIT_FAILURE;
	} 
	
	init_globals();
	
	printf("Globals Good, spawning threads\n");
	
	// Spawn In Threads
	int i;
	for (i = 0; i < nIn; i++) {
		if (pthread_create(&inThread[i], NULL, read_in, NULL)) {
			printf("Fatal Error: One of more IN threads could not be created\n");
			return EXIT_FAILURE;
		}

	}
/*
	// Spawn Work Threads 
	for (i = 0; i < nWork; i++) {
		if (pthread_create(&workThread[i], NULL, encrypt_data, NULL)) {
			printf("Fatal Error: One of more WORK threads could not be created\n");
			return EXIT_FAILURE;
		}
	}
	
	// Spawn Out Threads 
	for (i = 0; i < nOut; i++) {
		if (pthread_create(&outThread[i], NULL, write_out, NULL)) {
			printf("Fatal Error: One of more OUT threads could not be created\n");
			return EXIT_FAILURE;
		}
	}
	*/
	// Wait for In Threads
	for (i = 0; i < nIn; i++) {
		pthread_join(inThread[i], NULL);
		//inThreadsCompleted++;	// no need to worry about sync issues here
								// it may make the work threads run slightly longer
								// but not too much longer.
	}
	
	// The file has now been completely read into memory and is awaiting encryption
	file_read = 1;		
	printf("file read %d\n", unprocessedQ.size);
	
	// Wait for Work Threads
	for (i = 0; i < nWork; i++) {
		pthread_join(workThread[i], NULL);
	}
	
	// The file has now been completely encrypted and is awaiting write to disk
	file_processed = 1;
	
	// Wait for Out Threads
	for (i = 0; i < nOut; i++) {
		pthread_join(outThread[i], NULL);
	}
		
	return EXIT_SUCCESS;
}

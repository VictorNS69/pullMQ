/*
 * Incluya en este fichero todas las definiciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>		
#include <string.h>		
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <netdb.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#define CREATE 1
#define DESTROY 2
#define PUT 3
#define GET 4

typedef struct{
	int operation;
	size_t queue_name_len;
	char *queue_name;
	size_t msg_len;
	void *msg;
	bool blocking;
} Request;

struct Node{
    char *msg;
    struct Node *next;
};

typedef struct{
    char *name;
    struct Node *start;
    struct Node *end;
    unsigned int size;
} FIFO;

typedef struct {
    FIFO *list;
    int size;
} Array;

//FIFO 
void printFifo(FIFO *fifo);
void initFifo(FIFO *fifo, const char *name);
char *push(FIFO *list, char *msg);
char *pop(FIFO *list);
//Dynamic Array
void initArray(Array *array);
char* insertArray(Array *array, FIFO fifo);
void freeArray(Array *array);
int deleteArray(Array *array, FIFO fifo);
void printArray(Array *array);
int indexOf(Array *array, const char *name);

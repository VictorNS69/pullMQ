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
    size_t size;
    struct Node *next;
};

typedef struct{
    char *name;
    struct Node *first; //start
    struct Node *last; //end
    int *awaiting;
    int n_awaiting;
} Queue; // FIFO

typedef struct {
    Queue *array; //FIFO *list
    int size;
} Queues; // Array


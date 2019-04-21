#include "comun.h"
#include "pullMQ.h"

#include <stdio.h>
#include <stdlib.h>

// BROKER_HOST: nombre de la máquina donde ejecuta el broker.
// BROKER_PORT: número de puerto TCP por el que está escuchando.

struct Node {
	char *msg;
	struct Node *next;
};

struct Queue {
	char *queues;
	unsigned int size;
	struct Node *first;
	struct Node *last;
	bool blocking;
};

/* 0 si la operación se realizó satisfactoriamente y
 * un valor negativo en caso contrario
 * cola: nombre de la cola
 */
int createMQ(const char *cola) {
	printf("Creando cola");
	struct Queue *q;
	q->first = NULL;
	q->last = NULL;
	q->size = 0;
	return 0;
}

/* 0 si la operación se realizó satisfactoriamente y
 * un valor negativo en caso contrario
 * cola: nombre de la cola
 */
int destroyMQ(const char *cola){
	return 0;
}

/* 0 si la operación se realizó satisfactoriamente y
 * un valor negativo en caso contrario
 * cola: nombre de la cola
 * mensaje: mensaje
 * tam: tamaño del mensaje
 */
int put(const char *cola, const void *mensaje, size_t tam){
	return 0;
}

/* 0 si la operación se realizó satisfactoriamente y
 * un valor negativo en caso contrario
 * cola: nombre de la cola
 * mensaje: mensaje
 * tam: tamaño del mensaje
 * blocking: indica si la cola es boqueante
 */
int get(const char *cola, void **mensaje, size_t *tam, bool blocking){
	return 0;
}

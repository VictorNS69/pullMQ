#include "comun.h"
#include "pullMQ.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// BROKER_HOST: nombre de la máquina donde ejecuta el broker.
// BROKER_PORT: número de puerto TCP por el que está escuchando.

bool initialized = false;
Array a;

/* 0 si la operación se realizó satisfactoriamente y
 * un valor negativo en caso contrario
 * cola: nombre de la cola
 */
int createMQ(const char *cola) {
	if(initialized == false){
			initArray(&a);
			initialized = true;
	}
	// La cola existe
	int index = indexOf(&a, cola);
	if (index != -1){
		return -1;
	}
	// insertar la nueva cola
	FIFO fifo;
	initFifo(&fifo, cola);
	insertArray(&a, fifo);
	return 0;
	}

/* 0 si la operación se realizó satisfactoriamente y
* un valor negativo en caso contrario
* cola: nombre de la cola
*/
int destroyMQ(const char *cola){
	int index = indexOf(&a, cola);
	if (index == -1){
		return -1;
	}
	return deleteArray(&a, a.list[index]);
}

/* 0 si la operación se realizó satisfactoriamente y
 * un valor negativo en caso contrario
 * cola: nombre de la cola
 * mensaje: mensaje
 * tam: tamaño del mensaje
 */
int put(const char *cola, const void *mensaje, size_t tam){
	int index = indexOf(&a, cola);
	if (index == -1){
		return -1;
	}
	FIFO fifo = a.list[index];
	push(&fifo, (char*) mensaje);
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
	int index = indexOf(&a, cola);
	printf("COLA: %s\n", cola);
	printf("INDEX: %d,\n", index);
	if (index == -1){
		return -1;
	}
	FIFO fifo = a.list[index];
	printf("fifo name: %s\n", fifo.name);
	printf("fifo start: %s\n", fifo.start);
	printf("fifo end: %s\n", fifo.end);
	printf("LISTA: \n");
	printFifo(&fifo);
	int pope = pop(&fifo,  (char **) mensaje);
	printf("Valor de pop: %d\n", pope);
	a.list[index] = fifo;
	return pope;
}

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
	push(&a.list[index], (char*) mensaje);
	/*
	if (a.list->size == 2){
		printf("En if\n");
		printf("Posicion en array: %d\n", index);
		printf("Nombre de cola: %s\n", a.list[index].name);
		printf("Valor ANTERIOR: %s\n", a.list[index].end->msg);
	}
	printf("Posicion en array: %d\n", index);
	printf("Nombre de cola: %s\n", a.list[index].name);
	printf("Valor insertado: %s\n", a.list[index].start->msg);
	*/
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
	if (index == -1){
		//printf("Pasando en if 1\n");
		return -1;
	}
	if (a.list[index].size == 0){
		//printf("Pasando en if 2\n");
		return -1;
	}
	/*
	printf("Posicion en array: %d\n", index);
	printf("Tam de la cola: %d\n", a.list[index].size);
	printf("Nombre de cola: %s\n", a.list[index].name);
	printf("Valor sacado: %d\n", *a.list[index].start->msg);
	*/
	*mensaje = pop(&a.list[index]);
	//printf("sacado %s\n", (char *) *mensaje);
	return 0;
}

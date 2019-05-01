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
			a.list = (FIFO *)malloc(0);
			a.size = 0;
			initialized = true;
		}
		// Comprobar que no haya colas con ese nombre
		for(int i = 0; i < a.size; i++){
			if(strcmp(a.list[i].name, cola) == 0){
				return -1;
			}
		}
		// Reservar espacio para el nuevo elemento
		a.size++;
		a.list = (FIFO *)realloc(a.list, a.size * sizeof(a.list));
		if (a.list == NULL){
			return -1;
		}
		// Crear la cola
		FIFO queue;
		// Meter la cola en el array
		a.list[a.size - 1] = queue;
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

int main(){
	printf("libpullMQ\n");
}

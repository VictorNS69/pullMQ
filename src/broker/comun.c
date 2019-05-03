/*
 * Incluya en este fichero todas las implementaciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */
#include "comun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

///////////////////////// FIFO /////////////////////////
/** Prints the list of elements
 *  fifo: a FIFO list
 */
void printFifo(FIFO *fifo){
    if(fifo->start != NULL){
      struct Node *n = fifo->start;   
      while (n->next != NULL){
          printf(" %s ", n->msg);
          n = n->next;
      }
      printf(" %s \n", n->msg);
    }
}

/** Initializes the FIFO
 *  fifo: FIFO list to initialize
 */
void initFifo(FIFO *fifo, const char *name){
    fifo->name = (char *)malloc(strlen(name));
    strcpy(fifo->name, name);
    fifo->start = NULL;
    fifo->end = NULL;
    fifo->size = 0;
}

/** Pushes a string into the FIFO
 *  list: FIFO list
 *  msg: string to insert
 *  returns char*: the inserted element
 */
char *push(FIFO *list, char *msg){
    struct Node *node = malloc(sizeof(struct Node));
    node->msg = msg;
    if (list->end != NULL)
        list->end->next = node;

    list->end = node;
    if (list->start == NULL)
        list->start = node;

    list->size++;
    return list->end->msg;
}

/** Remove the first element of the FIFO
 *  list: FIFO list
 *  returns int: 0 if success, -1 if not
 */
int pop(FIFO *list, char **msg){
    if (list->end == NULL){
        return -1;
    }
    struct Node *start = list->start;
    *msg = start->msg;

    struct Node **second;
    struct Node *current = list->end;

    do {
        if(current->next == start) {
            *second = current;
            return 0;
        }
    } while((current = current->next) != NULL);
    second->next = NULL;
    list->start = second;

    free(start);
    /*char *msg = list->start->msg;
    list->start = list->start->next != NULL ? list->start->next : NULL;
    list->size--;
    return msg;*/
}
///////////////////////// FIFO /////////////////////////

///////////////////////// DYNA /////////////////////////

/** Initializes the array
 *  array: the array of Strings
 */
void initArray(Array *array) {
  array->list = malloc(0);
  array->size = 0;
}

/** Insert a new fifo to the end of the array
 *  array: the array
 *  fifo: the new fifo
 *  returns the name of the inserted fifo
 */
char* insertArray(Array *array, FIFO fifo) {
    array->size++;
    array->list = realloc(array->list, array->size * sizeof(*array->list));
    initFifo(&fifo, fifo.name);
    array->list[array->size -1] = fifo;
    return array->list[array->size -1].name;
}

/** Destroys an array
 *  array: the array
 */
void freeArray(Array *array) {
  free(array->list);
  array->list = NULL;
}

/** Delete a fifo of the array
 *  array: array
 *  fifo: fifo to remove
 *  returns int: 0 if success, -1 if not
 */
int deleteArray(Array *array, FIFO fifo){
  int index = indexOf(array, fifo.name);
  if (index == -1){
    return -1;
  }
  FIFO newFifo = array->list[index];
  array->size--;
  FIFO *temp = malloc(array->size * sizeof(*array->list));
  memmove(
        temp,
        array->list,
        (index + 1) * sizeof(*array->list));
  memmove(
        temp + index,
        array->list + index + 1,
        (array->size - index) * sizeof(*array->list));
  free(array->list);
  array->list = temp;
  return 0;
}

/** Prints the array
 *  array: array
 */
void printArray(Array *array){
    for (int i = 0; i < array->size; i++)
        printf("%s", array->list[i].name);
    printf("\n");
}

/** Retrieves the index of the fifo "name"
 *  array: array
 *  char: name
 *  returns int: index if success, -1 if not
 */
int indexOf(Array *array, const char *name){
	for(int i = 0; i < array->size; i++){	
		if(strcmp(array->list[i].name, name) == 0){
			return i;
		}
	}
	return -1;
}
///////////////////////// DYNA /////////////////////////

/*
void main(){
    printf("comun.o\n");
    printf("////////// FIFO //////////\n");
    FIFO list;
    initFifo(&list, "fifo");
    printf("+\tInsertado %s\n", push(&list, "elem1"));
    printf("+\tInsertado %s\n", push(&list, "elem2"));
    printf("Lista: ");
    printFifo(&list);
    printf("+\tInsertado %s\n", push(&list, "elem3"));
    printf("+\tInsertado %s\n", push(&list, "elem4"));
    printf("Lista: ");
    printFifo(&list);
    printf("-\tEliminado %s\n", pop(&list));
    printf("+\tInsertado %s\n", push(&list, "elem5"));
    printf("Lista: ");
    printFifo(&list);
    printf("////////// ARRAY //////////\n");
    Array array;
    initArray(&array);
    FIFO elem1;
    initFifo(&elem1, "elem1");
    printf("+\tInsertado %s\n", insertArray(&array, elem1));
    FIFO elem2;
    initFifo(&elem2, "elem2");
    printf("+\tInsertado %s\n", insertArray(&array, elem2));
    FIFO elem3;
    initFifo(&elem3, "elem3");
    printf("+\tInsertado %s\n", insertArray(&array, elem3));
    printf("Array: ");
    printArray(&array);
    printf("-\tEliminado %d\n", deleteArray(&array, elem2));
    printf("Array: ");
    printArray(&array);
    printf("+\tInsertado %s\n", insertArray(&array, elem2));
    printf("Array: ");
    printArray(&array);
    printf("-\tEliminado %d\n", deleteArray(&array, elem2)); 
    printf("-\tEliminado %d\n", deleteArray(&array, elem2)); // -1 porque ya ha sido borrado
    printf("Array: ");
    printArray(&array);
    printf("Valor en 1: %s\n", array.list[1].name);
}
*/
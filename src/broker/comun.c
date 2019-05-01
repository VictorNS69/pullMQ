/*
 * Incluya en este fichero todas las implementaciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */
#include "comun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node{
    char *msg;
    struct Node *next;
};

typedef struct{
    struct Node *start;
    struct Node *end;
    unsigned int size;
} FIFO;

typedef struct {
  char **string;
  int size;
} Array;

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
void initFifo(FIFO *fifo){
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
 *  returns char*: the element removed from the list
 */
char *pop(FIFO *list){
    char *msg = list->start->msg;
    list->start = list->start->next != NULL ? list->start->next : NULL;
    list->size--;
    return msg;
}
///////////////////////// FIFO /////////////////////////

///////////////////////// DYNA /////////////////////////
/** Initializes the array
 *  array: the array of Strings
 */
void initArray(Array *array) {
  array->string = malloc(0);
  array->size = 0;
}

/** Insert a new element to the end of the array
 *  array: the array
 *  element: the new element
 *  returns int: 0 if success, -1 if not
 */
int insertArray(Array *array, char *element) {
    array->size++;
    array->string = (char **)realloc(array->string, array->size * sizeof(*array->string));
    array->string[array->size - 1] = (char *)malloc(strlen(element) * sizeof(*array->string[array->size - 1]));
    if (!array->string[array->size - 1]){
        return -1;
    }
    strcpy(array->string[array->size - 1], element);
    return 0;
}

/** Destroys an array
 *  array: the array
 */
void freeArray(Array *array) {
  free(array->string);
  array->string = NULL;
}

/** Delete an element of the array
 *  array: array
 *  element: element to remove
 *  returns int: 0 if success, -1 if not
 */
int deleteArray(Array *array, char *element){
  int i;
  for (i = 0; i < array->size; i++){
    if(strcmp(element,array->string[i])== 0){
      array->size--;
      char **temp = (char **)malloc(array->size * sizeof(*array->string));
      memmove(
        temp,
        array->string,
        (i + 1) * sizeof(*array->string));
      memmove(
        temp + i,
        array->string + i + 1,
        (array->size - i) * sizeof(*array->string));
      free(array->string[i]);
      array->string = temp;
      return 0;
    }
  }
  return -1;
}

/** Prints the array
 *  list: array
 */
void printArray(Array *array){
    printf("Tamaño: %d \n", array->size);
    for (int i = 0; i < array->size; ++i)
        printf("%s ", array->string[i]);
    printf("\n");
}
///////////////////////// DYNA /////////////////////////

void main(){
    printf("////////// FIFO //////////\n");
    FIFO list;
    initFifo(&list);
    printf("+\tInsertado %s\n", push(&list, "elem1"));
    printf("+\tInsertado %s\n", push(&list, "elem2"));
    printf("Lista: ");
    printFifo(&list);
    //printf("-\tEliminado %s\n", pop(&list));
    printf("+\tInsertado %s\n", push(&list, "elem3"));
    printf("+\tInsertado %s\n", push(&list, "elem4"));
    printf("Lista: ");
    printFifo(&list);
    printf("-\tEliminado %s\n", pop(&list));
    printf("Lista: ");
    printFifo(&list);
    printf("////////// ARRAY //////////\n");
}
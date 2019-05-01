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
  array->list = malloc(0);
  array->size = 0;
}

/** Insert a new element to the end of the array
 *  array: the array
 *  element: the new element
 *  returns int: 0 if success, -1 if not
 */
char* insertArray(Array *array, FIFO fifo) {
    printf("INSERTAR\n");
    printf("Tamaño: %d\n", array->size);
    for(int i = 0; i < array->size; i++){
      printf("i %d\n", i);
      //printf("nombre: %s\n", array[i].list->name);
      /*if (array[i].list == NULL){
        //printf("HUECO NULL. i=%d\n", i );
        array[i].list->name = fifo.name;
        array[i].list->start = fifo.start;
        array[i].list->end = fifo.end;
        return array[i].list->name;
      }
      else */
      //printf("%s",array[i].list->name);
      if(strcmp(array[i].list->name, fifo.name) == 0){
        //printf("EXISTE i=%d\n", i );
        return "EXSIST";
      }
    }
    printf("NO HUECO NULL Y NO EXISTE\n");
    array->size++;
    printf("PASO\n");
    array->list = realloc(array->list, array->size * sizeof(*array->list));
    array[array->size -1].list->name = fifo.name;
    array[array->size -1].list->start = fifo.start;
    array[array->size -1].list->end = fifo.end;
    return array[array->size -1].list->name;
}

/** Destroys an array
 *  array: the array
 */
void freeArray(Array *array) {
  free(array->list);
  array->list = NULL;
}

/** Delete an element of the array
 *  array: array
 *  element: element to remove
 *  returns int: 0 if success, -1 if not
 */
int deleteArray(Array *array, FIFO fifo){
  printf("NOMBRE para borrar: %s\n", fifo.name);
  //printArray(&array);
  int i;
  bool find = false;
  Array newArray;
  initArray(&newArray);
  for (i = 0; i < array->size; i++){
    printf("i %d\n", i);
    if(strcmp(fifo.name,array[i].list->name) == 0){
      printf("Encontrado elemento\n");
      find = true;
      continue;
    }
    FIFO newFifo = *array[i].list;
    insertArray(&newArray, newFifo);
    //printArray(&newArray);
    printf("insertado\n");
    /*
    if(strcmp(fifo.name,array[i].list->name)== 0){
      FIFO *temp = malloc(array->size * sizeof(*array->list));
      memmove(
        temp,
        array->list,
        (i + 1) * sizeof(*array->list));
      memmove(
        temp + i,
        array->list + i + 1,
        (array->size - i) * sizeof(*array->list));
      free(array[i].list);
      array->list = temp;
      return 0;
    }*/
  }
  array->list = newArray.list;
  array->size = newArray.size;
  return find ? 0: -1;
}

/** Prints the array
 *  list: array
 */
void printArray(Array *array){
    //printf("Tamaño: %d \n", array->size);
    for (int i = 0; i < array->size; ++i)
        printf("%s ", array[i].list->name);
    printf("\n");
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
    //printf("-\tEliminado %s\n", pop(&list));
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
    /*printf("Array: ");
    printArray(&array);
    printf("+\tInsertado %s\n", insertArray(&array, elem2));
    printf("Array: ");
    printArray(&array);
    //printf("-\tEliminado %d\n", deleteArray(&array, elem2)); // -1 porque ya ha sido borrado
    //printf("Array: ");
    //printArray(&array);
    //printf("Valor en 1: %s\n", array.list[1]);
    */
//}
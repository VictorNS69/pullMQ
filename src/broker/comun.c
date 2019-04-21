/*
 * Incluya en este fichero todas las implementaciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */
#include "comun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
  char **array;
  int size;
} Array;

void initArray(Array *a, int initialSize) {
  a->array = malloc(initialSize);
  a->size = initialSize;
}

/** 0 Si exito
 *  -1 si no exito
 */
int insertArray(Array *a, char *element) {
    a->size++;
    a->array = (char **)realloc(a->array, a->size * sizeof(*a->array));
    a->array[a->size - 1] = (char *)malloc(strlen(element) * sizeof(*a->array[a->size - 1]));
    if (!a->array[a->size - 1]){
        return -1;
    }
    strcpy(a->array[a->size - 1], element);
    return 0;
}

void freeArray(Array *a) {
  free(a->array);
  a->array = NULL;
}

int eliminarArray(Array *a, char *element){
  int i;
  for (i = 0; i <= a->size; i++){
    if(strcmp(element,a->array[i])== 0){
      a->size--;
      char **temp = (char **)malloc(a->size * sizeof(*a->array));
      memmove(
        temp,
        a->array,
        (i + 1) * sizeof(*a->array));
      memmove(
        temp + i,
        a->array + i + 1,
        (a->size - i) * sizeof(*a->array));
      free(a->array[i]);
      a->array = temp;
      return 0;
    }
  }
  return -1;
}

void printArray(Array *list){
    printf("Tamaño: %d \n", list->size);
    for (int i = 0; i < list->size; ++i)
        printf("%s ", list->array[i]);
    printf("\n");
}

int main(){
    Array arr;
    initArray(&arr, 0);
    int i;
    char n [4];
    for (i = 0; i < 50;i++){
      sprintf(n,"%d",i);
      insertArray(&arr, n);
    }
    printArray(&arr);
    sprintf(n,"%d",1);
    eliminarArray(&arr, n);
    printArray(&arr);
    //printf("%s \n", arr.array[0]);
    //freeArray(&arr);
}
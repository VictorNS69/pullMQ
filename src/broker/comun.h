/*
 * Incluya en este fichero todas las definiciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */

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
int pop(FIFO *list, char **msg);
//Dynamic Array
void initArray(Array *array);
char* insertArray(Array *array, FIFO fifo);
void freeArray(Array *array);
int deleteArray(Array *array, FIFO fifo);
void printArray(Array *array);
int indexOf(Array *array, const char *name);

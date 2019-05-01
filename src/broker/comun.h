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

void printFifo(FIFO *fifo);
void initFifo(FIFO *fifo, const char *name);
char *push(FIFO *list, char *msg);
char *pop(FIFO *list);



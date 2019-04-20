#include <stdio.h>
#include "comun.h"

/* incluye toda la funcionalidad del sistema, implementando las operaciones
 * descritas en el apartado previo y gestionando las colas de mensajes.
 * Recibe como argumento el número del puerto por el que prestará servicio.
*/
int main(int argc, char *argv[]){
    if(argc!=2) {
        fprintf(stderr, "Uso: %s puerto\n", argv[0]);
        return 1;
    }
    return 0;
}

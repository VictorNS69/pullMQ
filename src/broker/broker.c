#include <stdio.h>
#include "comun.h"

int main(int argc, char *argv[]){
    if(argc!=2) {
        fprintf(stderr, "Uso: %s puerto\n", argv[0]);
        return 1;
    }
    return 0;
}
#include "stdio.h"

/*Agregar el extern del ensamblador*/
extern int float_to_int_asm(float);

// Funcion para castear un numero flotante a entero
int float_to_int(float indice_gini){
    // Delegamos la lógica matemática a nuestra función nativa en NASM
    return float_to_int_asm(indice_gini);
}
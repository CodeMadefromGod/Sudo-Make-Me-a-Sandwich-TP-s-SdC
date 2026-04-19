#include "stdio.h"


/*Agregar el extern del ensamblador*/

// Funcion para castear un numero flotante a entero
int float_to_int(float indice_gini){
    int indice_gini_int = (int)indice_gini;
    return indice_gini_int + 1; // Se suma 1 para redondeo
}
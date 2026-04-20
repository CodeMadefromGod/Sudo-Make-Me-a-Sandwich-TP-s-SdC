#include <stdio.h>
#include <stdlib.h>

// Declaramos la función externa desarrollada en NASM
extern int float_to_int_asm(float);

int main() {
    FILE *file = fopen("gini_data.txt", "r");
    if (file == NULL) {
        printf("Error: No se pudo abrir 'gini_data.txt'.\n");
        printf("Por favor, ejecuta 'python3 api_Rest.py' primero para descargar y generar los datos.\n");
        return 1;
    }

    // 1. Contar exactamente cuántos elementos tiene el archivo
    int n_elements = 0;
    float val;
    while (fscanf(file, "%f", &val) == 1) {
        n_elements++;
    }

    if (n_elements == 0) {
        printf("No se encontraron registros en el archivo.\n");
        fclose(file);
        return 1;
    }

    // 2. Volver al inicio del archivo para leer los datos reales
    rewind(file);

    // 3. Reservar dinámicamente la memoria JUSTA que necesitamos
    float *test_vals = (float *)malloc(n_elements * sizeof(float));
    int *resultados = (int *)malloc(n_elements * sizeof(int));

    if (test_vals == NULL || resultados == NULL) {
        printf("Error al reservar memoria dinámica.\n");
        fclose(file);
        return 1;
    }

    // 4. Llenar nuestro array dinámico
    int index = 0;
    while (fscanf(file, "%f", &val) == 1 && index < n_elements) {
        test_vals[index] = val;
        index++;
    }
    fclose(file);

    printf("=========================================\n");
    printf("[GDB Standalone] Ejecutable Puro en C\n");
    printf("Procesando un array de %d elementos (reserva dinámica)...\n\n", n_elements);

    // Iteramos sobre el array 
    for (int i = 0; i < n_elements; i++) {
        resultados[i] = float_to_int_asm(test_vals[i]);
        printf("Índice original [%d]: %.2f  --> Resultado ASM (+1): %d\n", i, test_vals[i], resultados[i]);
    }
    
    printf("=========================================\n");
    
    // 5. Liberar la memoria reservada para no generar leaks
    free(test_vals);
    free(resultados);

    return 0;
}
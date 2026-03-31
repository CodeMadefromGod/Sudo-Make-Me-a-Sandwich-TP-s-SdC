#include <stdio.h>

#define VALUE_TXT "last_gini_index.txt"

FILE *file = NULL;
float gini_index = 0.0;
int int_value = 0;

extern int float_to_int(float value);

int main(){
    file = fopen(VALUE_TXT, "r");
    
    if(file == NULL){
        printf("Error opening file\n");
        return -1;
    }
    
    fscanf(file, "%f", &gini_index);
    printf("Gini Index: %f\n", gini_index);
    fclose(file);

    int_value = float_to_int(gini_index);
    printf("Integer Value: %d\n", int_value);
    
    return 0;
}
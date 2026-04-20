CC = gcc
CFLAGS = -g -Wall -fPIC

NASM = nasm
NASMFLAGS = -f elf64 -g

# Regla principal que construye ambos targets
all: libgini.so debug_gini

# 1. Compilar el archivo Ensamblador a código objeto (.o)
float_to_int_asm.o: float_to_int_asm.asm
	$(NASM) $(NASMFLAGS) float_to_int_asm.asm -o float_to_int_asm.o

# 2. Compilar la librería compartida para Python (usa float_to_int.c como wrapper)
libgini.so: float_to_int.c float_to_int_asm.o
	$(CC) -shared $(CFLAGS) -o libgini.so float_to_int.c float_to_int_asm.o

# 3. Compilar el ejecutable estático C puro para depurar con GDB
debug_gini: main.c float_to_int_asm.o
	$(CC) $(CFLAGS) -o debug_gini main.c float_to_int_asm.o

# Limpieza de binarios generados
clean:
	rm -f *.o *.so debug_gini

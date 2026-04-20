#!/bin/bash

# Terminar la ejecución si algún comando falla
set -e

echo "==> Configurando el entorno para el TP2..."

# Crear el entorno virtual si no existe
if [ ! -d ".venv" ]; then
    echo "==> Creando entorno virtual en TP2/.venv..."
    python3 -m venv .venv
fi

# Activar el entorno e instalar dependencias
echo "==> Activando entorno virtual e instalando 'requests'..."
source .venv/bin/activate
pip install requests

# Usar el Makefile para compilar la librería (.so) y el entorno GDB
echo "==> Compilando código C y NASM mediante Makefile..."
make clean
make all

echo "==> ¡Setup completado con éxito!"
echo "==> Ejecutando api_Rest.py:"
echo "======================================================="

# Ejecutar el script en Python
python3 api_Rest.py

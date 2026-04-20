import requests
import ctypes
import matplotlib.pyplot as plt
import time

results = []
years = []
last_gini_index = None
file = None

response = requests.get('https://api.worldbank.org/v2/en/country/all/indicator/SI.POV.GINI?format=json&date=2000:2026&per_page=32500&page=1&country=%22Argentina%22')
if response:
    print("OK")
else:
    print("Error:", response.status_code)

print()
      
data = response.json()
not_metadata = data[1]

for i in not_metadata:
    if i["country"]["value"] == "Argentina":
        if i["value"] is not None: 
            print("Gini index from Argentina in", i["date"], "is:", i["value"])
            results.append(i["value"])
            years.append(i["date"])

print()
print(results)
print()
print(years)

if len(results) == 0:  
    print("No Gini index data available for Argentina.")
    exit()

print()
print("Guardando datos en gini_data.txt para GDB...")
with open("gini_data.txt", "w") as f:
    for val in results:
        f.write(f"{val}\n")

# Cargar la libreria compartida
lib = ctypes.CDLL("./libgini.so")

# Definimos los argumentos de la funcion y el tipo de retorno
lib.float_to_int.argtypes = [ctypes.c_float]
lib.float_to_int.restype = ctypes.c_int

start = time.time()

# Procesamos TODO el array dinámicamente llamando a la función en ASM
resultados_enteros = []
for val in results:
    entero_redondeado = lib.float_to_int(float(val))
    resultados_enteros.append(entero_redondeado)

end = time.time()

print()
print("Array original de índices Gini obtenidos (float):")
print(results)
print()
print("Array resultante procesado por ASM (+1 redondeo):")
print(resultados_enteros)

# Graficamos los datos
anios_enteros = [int(a) for a in years]

plt.figure(figsize=(30, 5))
plt.plot(anios_enteros, resultados_enteros, marker='o', linewidth=2)

plt.title('Índice GINI en Argentina', fontsize=14, fontweight='bold')
plt.xlabel('Año', fontsize=12)
plt.ylabel('GINI', fontsize=12)

plt.grid(True, linestyle='--', alpha=0.5)
plt.xticks(anios_enteros)
plt.tight_layout()

plt.show()

print(end-start)
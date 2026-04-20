import requests
import ctypes

results = []
last_gini_index = None
file = None

response = requests.get('https://api.worldbank.org/v2/en/country/all/indicator/SI.POV.GINI?format=json&date=2016:2026&per_page=32500&page=1&country=%22Argentina%22')
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

print()
print(results)

if len(results) == 0:  
    print("No Gini index data available for Argentina.")
    exit()

print("Guardando datos en gini_data.txt para GDB...")
with open("gini_data.txt", "w") as f:
    for val in results:
        f.write(f"{val}\n")

# Cargar la libreria compartida
lib = ctypes.CDLL("./libgini.so")

# Definimos los argumentos de la funcion y el tipo de retorno
lib.float_to_int.argtypes = [ctypes.c_float]
lib.float_to_int.restype = ctypes.c_int

# Procesamos TODO el array dinámicamente llamando a la función en ASM
resultados_enteros = []
for val in results:
    entero_redondeado = lib.float_to_int(float(val))
    resultados_enteros.append(entero_redondeado)

print()
print("Array original de índices Gini obtenidos (float):")
print(results)
print()
print("Array resultante procesado por ASM (+1 redondeo):")
print(resultados_enteros)
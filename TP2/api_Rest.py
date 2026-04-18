import requests
import ctypes

results = []
last_gini_index = None
file = None

response = requests.get('https://api.worldbank.org/v2/en/country/all/indicator/SI.POV.GINI?format=json&date=2011:2020&per_page=32500&page=1&country=%22Argentina%22')
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

last_gini_index = results[0]
last_gini_index = float(last_gini_index)
print()
print("The last Gini index from Argentina is:", last_gini_index)

# Cargar la libreria compartida
lib = ctypes.CDLL("./libgini.so")

# Definimos los argumentos de la funcion y el tipo de retorno

lib.float_to_int.argtypes = [ctypes.c_float]

lib.float_to_int.restype = ctypes.c_int

# Llamamos a la funcion y hacemos la conversion
resultado_final = lib.float_to_int(last_gini_index)
print()
print("The last Gini index from Argentina rounded to the nearest integer is:", resultado_final)
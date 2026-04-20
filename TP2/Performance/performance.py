import ctypes
import time
import requests

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

if len(results) == 0:  
    print("No Gini index data available for Argentina.")
    exit()

print()
print("Guardando datos en gini_data.txt para GDB...")
with open("gini_data.txt", "w") as f:
    for val in results:
        f.write(f"{val}\n")

# Performance Python
start = time.time()

for _ in range(10000):
    for val in results:
        _ = int(val + 1) 

end = time.time()

print("Python:", end - start)


# Performance Python + C
lib_c = ctypes.CDLL("./lib_c.so")
lib_c.float_to_int.argtypes = [ctypes.c_float]
lib_c.float_to_int.restype = ctypes.c_int

start = time.time()

for _ in range(10000):
    for val in results:
        _ = lib_c.float_to_int(val) 

end = time.time()

print("Python + C:", end - start)

# Performance Python + C + ASM
lib_asm = ctypes.CDLL("./lib_asm.so")
lib_asm.float_to_int.argtypes = [ctypes.c_float]
lib_asm.float_to_int.restype = ctypes.c_int

start = time.time()

for _ in range(10000):
    for val in results:
        _ = lib_asm.float_to_int(val) 

end = time.time()
print("Python + C + ASM:", end - start)
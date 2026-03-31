import requests

results = []
last_gini_index = None
file = None

response = requests.get('https://api.worldbank.org/v2/en/country/all/indicator/SI.POV.GINI?format=json&date=2011:2020&per_page=32500&page=1&country=%22Argentina%22')
if response:
    print("OK")
else:
    print("not OK")

print()
      
data = response.json()
not_metadata = data[1]

for i in not_metadata:
    if i["country"]["value"] == "Argentina":
        print("Gini index from Argentina in", i["date"], "is:", i["value"])
        results.append(i["value"])

print()
print(results)

last_gini_index = results[0]
print()
print("The last Gini index from Argentina is:", last_gini_index)

# Save the last Gini index to a text file
with open("last_gini_index.txt", "w") as file:
    file.write(str(last_gini_index))


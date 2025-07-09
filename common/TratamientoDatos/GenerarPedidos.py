import pandas as pd
import random
import json
from collections import defaultdict

# Leer el CSV del restaurante
df = pd.read_csv("DataRestaurante.csv")

# Mapeo manual de item_name a IDs de platos
item_to_id = {
    "Aalopuri": 1,
    "Vadapav": 2,
    "Sugarcane juice": 3,
    "Panipuri": 4,
    "Frankie": 5,
    "Sandwich": 6,
    "Cold coffee": 7
}

# Agrupar platos por pedido
pedidos = defaultdict(list)

for _, row in df.iterrows():
    order_id = row["order_id"]
    nombre_item = row["item_name"]
    cantidad = int(row["quantity"])

    if nombre_item in item_to_id:
        pedidos[order_id].append({
            "id": item_to_id[nombre_item],
            "cantidad": cantidad
        })

# Crear estructura de pedidos
estructura_final = []

for _, platos in pedidos.items():
    estructura_final.append({
        "COMANDO": "NUEVO_PEDIDO",
        "mesa": random.randint(1, 9),
        "id_recepcionista": random.randint(1, 10),
        "platos": platos
    })

# Guardar a JSON
with open("pedidos_generados.json", "w", encoding="utf-8") as f:
    json.dump(estructura_final, f, indent=2, ensure_ascii=False)

print("✅ Archivo 'pedidos_generados.json' generado correctamente.")

import pandas as pd
import json
import random
from collections import defaultdict

# Cargar el CSV
df = pd.read_csv("DataRestaurante.csv")

# Mapeo de nombre de platos a ID (ajusta si cambias tu sistema)
item_to_id = {
    "Aalopuri": 1,
    "Vadapav": 2,
    "Sugarcane juice": 3,
    "Panipuri": 4,
    "Frankie": 5,
    "Sandwich": 6,
    "Cold coffee": 7
}

# Nombres falsos para clientes si no hay dato
nombres_falsos = [
    "Luis", "Carlos", "Andrea", "Gabriela", "Jorge",
    "Lucía", "Miguel", "Valeria", "Ricardo", "Ana"
]

# Agrupar pedidos
pedidos_dict = defaultdict(list)
meta_info = {}

for _, row in df.iterrows():
    id_pedido = int(row["order_id"])
    item = str(row["item_name"])
    cantidad = int(row["quantity"])
    precio_unitario = float(row["item_price"])

    total_calculado = cantidad * precio_unitario
    total = float(row["transaction_amount"]) if not pd.isna(row["transaction_amount"]) else total_calculado

    if item in item_to_id:
        pedidos_dict[id_pedido].append({
            "id": item_to_id[item],
            "nombre": item,
            "cantidad": cantidad,
            "precio": precio_unitario
        })

        # Determinar nombre cliente
        recibido_por = str(row["received_by"]) if isinstance(row["received_by"], str) and row["received_by"].strip() else ""
        nombre_cliente = recibido_por if recibido_por else random.choice(nombres_falsos)

        # Guardar meta-info (una vez por pedido)
        if id_pedido not in meta_info:
            meta_info[id_pedido] = {
                "nombreCliente": nombre_cliente,
                "mesa": random.randint(1, 9),
                "total": total
            }

# Crear lista final
estructura_final = []

for id_pedido, platos in pedidos_dict.items():
    info = meta_info[id_pedido]
    estructura_final.append({
        "idPedido": id_pedido,
        "nombreCliente": info["nombreCliente"],
        "numeroMesa": info["mesa"],
        "total": info["total"],
        "platos": platos
    })

# Guardar como JSON
with open("historial_pedidos.json", "w", encoding="utf-8") as f:
    json.dump(estructura_final, f, indent=2, ensure_ascii=False)

print("✅ Archivo 'historial_pedidos.json' generado correctamente.")

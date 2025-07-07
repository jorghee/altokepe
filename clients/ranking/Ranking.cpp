#include "Ranking.h"

std::map<int, RegistroPedido> ranking;

void registrarVenta(const Pedido& pedido) {
    for (const Plato& p : pedido.platos) {
        if (ranking.find(p.id) == ranking.end()) {
            ranking[p.id] = {p.id, p.nombre, 1};
        } else {
            ranking[p.id].cantidadVendida++;
        }
    }
}

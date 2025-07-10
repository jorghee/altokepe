#include "ArbolBMas.h"

void ArbolBMas::insertarPedido(const RegistroPedido &pedido) {
    datos[pedido.idPedido] = pedido;
}

RegistroPedido* ArbolBMas::buscarPedido(int id) {
    auto it = datos.find(id);
    return (it != datos.end()) ? &it->second : nullptr;
}

QList<RegistroPedido> ArbolBMas::ultimosPedidos(int cantidad) {
    QList<RegistroPedido> lista;
    for (auto it = datos.rbegin(); it != datos.rend() && cantidad > 0; ++it, --cantidad) {
        lista.prepend(it->second);
    }
    return lista;
}

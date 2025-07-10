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
    auto it = datos.rbegin();
    while (it != datos.rend() && cantidad--) {
        lista.prepend(it->second);
        ++it;
    }
    return lista;
}

int ArbolBMas::obtenerUltimoId() const {
    if (datos.empty()) return 0;
    return datos.rbegin()->first;
}

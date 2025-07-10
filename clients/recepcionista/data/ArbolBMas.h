#pragma once
#include "RegistroPedido.h"
#include <QList>
#include <map>

class ArbolBMas {
public:
    void insertarPedido(const RegistroPedido &pedido);
    RegistroPedido* buscarPedido(int id);
    QList<RegistroPedido> ultimosPedidos(int cantidad);
    int obtenerUltimoId() const;  // ✅ ahora correctamente dentro de la clase

private:
    std::map<int, RegistroPedido> datos;
};

#pragma once
#include "RegistroPedido.h"
#include <QList>
#include <map>

class ArbolBMas {
public:
    void insertarPedido(const RegistroPedido &pedido);
    RegistroPedido* buscarPedido(int id);
    QList<RegistroPedido> ultimosPedidos(int cantidad);

private:
    std::map<int, RegistroPedido> datos;
};

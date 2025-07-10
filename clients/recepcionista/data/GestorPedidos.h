#pragma once
#include "ArbolBMas.h"
#include <QString>

class GestorPedidos {
public:
    void registrar(const RegistroPedido &pedido);
    RegistroPedido* consultar(int id);
    QList<RegistroPedido> ultimos(int n);

private:
    ArbolBMas arbol;
    void guardarFacturaEnArchivo(const RegistroPedido &pedido);
};

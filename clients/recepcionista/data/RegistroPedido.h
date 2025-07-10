#pragma once
#include <QString>
#include <QList>

struct PlatoPedido {
    int id;
    QString nombre;
    int cantidad;
    double precio;
};

struct RegistroPedido {
    int idPedido;
    QString nombreCliente;
    int numeroMesa;
    QList<PlatoPedido> platos;
    double total;
};

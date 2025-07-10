#include "GestorPedidos.h"
#include <QDir>
#include <QFile>
#include <QTextStream>

void GestorPedidos::registrar(const RegistroPedido &pedido) {
    arbol.insertarPedido(pedido);
    guardarFacturaEnArchivo(pedido);
}

RegistroPedido* GestorPedidos::consultar(int id) {
    return arbol.buscarPedido(id);
}

QList<RegistroPedido> GestorPedidos::ultimos(int n) {
    return arbol.ultimosPedidos(n);
}

void GestorPedidos::guardarFacturaEnArchivo(const RegistroPedido &pedido) {
    QDir directorio("facturas");
    if (!directorio.exists()) {
        QDir().mkdir("facturas");
    }

    QString ruta = QString("facturas/factura_%1.txt").arg(pedido.idPedido, 4, 10, QChar('0'));
    QFile archivo(ruta);

    if (archivo.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&archivo);
        out << "FACTURA DE PEDIDO\n";
        out << "--------------------------\n";
        out << "Pedido N°: " << pedido.idPedido << "\n";
        out << "Cliente: " << pedido.nombreCliente << "\n";
        out << "Mesa: " << pedido.numeroMesa << "\n\n";
        out << "PLATOS:\n";

        for (const auto &plato : pedido.platos) {
            out << "- " << plato.nombre << " (x" << plato.cantidad << "): S/ "
                << QString::number(plato.cantidad * plato.precio, 'f', 2) << "\n";
        }

        out << "\nTOTAL: S/ " << QString::number(pedido.total, 'f', 2) << "\n";
        out << "--------------------------\n";
        archivo.close();
    }
}

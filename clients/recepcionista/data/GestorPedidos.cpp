#include "GestorPedidos.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

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
    // Obtener ruta del ejecutable
    QString rutaBase = QCoreApplication::applicationDirPath();
    QDir dir(rutaBase);
    dir.cdUp(); // Subir de /build a /recepcionista
    dir.cd("facturas");

    // Si no existe la carpeta 'facturas', crearla
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Construir ruta final de archivo
    QString nombreArchivo = QString("factura_%1.txt").arg(pedido.idPedido, 4, 10, QChar('0'));
    QString rutaCompleta = dir.filePath(nombreArchivo);

    QFile archivo(rutaCompleta);
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

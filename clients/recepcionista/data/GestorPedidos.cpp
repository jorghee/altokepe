#include "GestorPedidos.h"
GestorPedidos gestorPedidos;

#include "RegistroPedido.h"
#include "ArbolBMas.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

void GestorPedidos::cargarDesdeHistorial() {
    // Ruta absoluta hacia ../data/historial_pedidos.json
    QString ruta = QCoreApplication::applicationDirPath() + "/../data/historial_pedidos.json";
    QFile archivo(ruta);

    if (!archivo.exists()) {
        qWarning() << "⚠️ No se encontró historial_pedidos.json en:" << ruta;
        return;
    }

    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "❌ No se pudo abrir historial_pedidos.json";
        return;
    }

    QByteArray datos = archivo.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(datos);

    if (!doc.isArray()) {
        qWarning() << "❌ El archivo JSON no es un array.";
        return;
    }

    QJsonArray array = doc.array();
    for (const QJsonValue &valor : array) {
        QJsonObject obj = valor.toObject();

        RegistroPedido pedido;
        pedido.idPedido = obj["idPedido"].toInt();
        pedido.nombreCliente = obj["nombreCliente"].toString();
        pedido.numeroMesa = obj["numeroMesa"].toInt();
        pedido.total = obj["total"].toDouble();

        QJsonArray platosJson = obj["platos"].toArray();
        for (const QJsonValue &platoVal : platosJson) {
            QJsonObject pl = platoVal.toObject();
            PlatoPedido plato;
            plato.id = pl["id"].toInt();
            plato.nombre = pl["nombre"].toString();
            plato.cantidad = pl["cantidad"].toInt();
            plato.precio = pl["precio"].toDouble();
            pedido.platos.append(plato);
        }

        arbol.insertarPedido(pedido);
        guardarFacturaEnArchivo(pedido);  // ✅ Generar factura al cargar
    }

    qDebug() << "✅ Historial de pedidos cargado:" << array.size() << "pedidos.";
}

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

int GestorPedidos::obtenerUltimoIdPedido() const {
    return arbol.obtenerUltimoId();
}

void GestorPedidos::guardarFacturaEnArchivo(const RegistroPedido &pedido) {
    QString rutaCarpeta = QCoreApplication::applicationDirPath() + "/../facturas";
    QDir directorio(rutaCarpeta);
    if (!directorio.exists()) {
        QDir().mkdir(rutaCarpeta);
    }

    QString rutaArchivo = rutaCarpeta + QString("/factura_%1.txt").arg(pedido.idPedido, 4, 10, QChar('0'));
    QFile archivo(rutaArchivo);

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

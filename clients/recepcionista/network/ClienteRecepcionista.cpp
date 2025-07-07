// ClienteRecepcionista.cpp
#include "ClienteRecepcionista.h"
#include <QJsonDocument>
#include <QDebug>

ClienteRecepcionista::ClienteRecepcionista(QObject *parent) : QObject(parent), socket(new QTcpSocket(this)) {}

void ClienteRecepcionista::conectarAlServidor(const QString &host, quint16 puerto) {
    socket->connectToHost(host, puerto);
    if (socket->waitForConnected(5555)) {
        qDebug() << "Conectado al servidor";

        // Identificarse
        QJsonObject msg{
            {"comando", "IDENTIFICARSE"},
            {"rol", "Recepcionista"},
            {"id", 101}
        };
        socket->write(QJsonDocument(msg).toJson(QJsonDocument::Compact) + "\n");
    } else {
        qDebug() << "Error al conectar con el servidor.";
    }
}

void ClienteRecepcionista::enviarNuevoPedido(int mesa, int idRecepcionista, const QJsonArray &platos) {
    QJsonObject pedido{
        {"comando", "NUEVO_PEDIDO"},
        {"mesa", mesa},
        {"id_recepcionista", idRecepcionista},
        {"platos", platos}
    };
    socket->write(QJsonDocument(pedido).toJson(QJsonDocument::Compact) + "\n");
}

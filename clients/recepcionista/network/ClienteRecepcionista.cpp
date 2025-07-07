#include "ClienteRecepcionista.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

ClienteRecepcionista::ClienteRecepcionista(QObject* parent) : QObject(parent) {
    connect(&socket, &QTcpSocket::readyRead, this, &ClienteRecepcionista::onDatosRecibidos);
}

void ClienteRecepcionista::conectarAlServidor() {
    socket.connectToHost("127.0.0.1", 5555);
    if (!socket.waitForConnected(2000)) {
        qWarning() << "No se pudo conectar al servidor";
    } else {
        qDebug() << "Conectado al servidor";
        enviarIdentificacion();
    }
}

void ClienteRecepcionista::enviarIdentificacion() {
    QJsonObject mensaje {
        {"comando", "IDENTIFICARSE"},
        {"rol", "Recepcionista"},
        {"id", 101}
    };
    socket.write(QJsonDocument(mensaje).toJson(QJsonDocument::Compact) + "\n");
}

void ClienteRecepcionista::enviarNuevoPedido(int mesa, int idRecepcionista, const QJsonArray& platos) {
    QJsonObject pedido {
        {"comando", "NUEVO_PEDIDO"},
        {"mesa", mesa},
        {"id_recepcionista", idRecepcionista},
        {"platos", platos}
    };
    socket.write(QJsonDocument(pedido).toJson(QJsonDocument::Compact) + "\n");
}

void ClienteRecepcionista::onDatosRecibidos() {
    while (socket.canReadLine()) {
        QByteArray data = socket.readLine();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            emit mensajeRecibido(doc.object());
        } else {
            qWarning() << "Error al parsear JSON recibido";
        }
    }
}

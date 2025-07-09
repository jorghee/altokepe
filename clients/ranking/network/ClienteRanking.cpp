#include "ClienteRanking.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QTcpSocket>

ClienteRanking::ClienteRanking(QObject* parent)
    : QObject(parent), m_socket(new QTcpSocket(this)) {
    
    connect(m_socket, &QTcpSocket::connected, this, &ClienteRanking::onConectado);
    connect(m_socket, &QTcpSocket::readyRead, this, &ClienteRanking::onDatosRecibidos);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &ClienteRanking::onError);
}

void ClienteRanking::conectar(const QString& host, quint16 puerto) {
    qDebug() << "Conectando al servidor" << host << "en el puerto" << puerto << "...";
    m_socket->connectToHost(host, 5555);
}

void ClienteRanking::onConectado() {
    QJsonObject mensaje;
    mensaje["comando"] = "IDENTIFICARSE";  // ⚠️ En mayúsculas
    mensaje["rol"] = "Ranking";

    QJsonDocument doc(mensaje);
    m_socket->write(doc.toJson(QJsonDocument::Compact) + "\n");

    qDebug() << "Cliente Ranking conectado e identificado.";
}

void ClienteRanking::onError(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError);
    qWarning() << "❌ Error al conectar con el servidor:" << m_socket->errorString();
}

void ClienteRanking::onDatosRecibidos() {
    m_buffer.append(m_socket->readAll());

    while (m_buffer.contains('\n')) {
        int pos = m_buffer.indexOf('\n');
        QByteArray linea = m_buffer.left(pos);
        m_buffer.remove(0, pos + 1);

        QJsonDocument doc = QJsonDocument::fromJson(linea);
        if (!doc.isObject()) {
            qWarning() << "JSON inválido recibido:" << linea;
            continue;
        }

        QJsonObject obj = doc.object();
        QString evento = obj.value("evento").toString();

        if (evento == "ACTUALIZACION_ESTADO_GENERAL") {
            QJsonObject data = obj.value("data").toObject();
            QJsonArray ranking = data.value("ranking").toArray();
            emit rankingActualizado(ranking);
        } else {
            qDebug() << "Evento desconocido recibido:" << evento;
        }
    }
}

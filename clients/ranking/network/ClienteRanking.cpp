#include "ClienteRanking.h"
#include <QJsonDocument>
#include <QDebug>

ClienteRanking::ClienteRanking(QObject* parent)
    : QObject(parent),
      m_socket(new QTcpSocket(this)),
      m_timer(new QTimer(this))  // Inicializamos el timer
{
    connect(m_socket, &QTcpSocket::connected, this, &ClienteRanking::onConectado);
    connect(m_socket, &QTcpSocket::readyRead, this, &ClienteRanking::onDatosRecibidos);
    connect(m_timer, &QTimer::timeout, this, &ClienteRanking::solicitarEstado);
}

void ClienteRanking::conectar(const QString& host, quint16 puerto) {
    m_socket->connectToHost(host, puerto);
}

void ClienteRanking::onConectado() {
    QJsonObject mensaje;
    mensaje["comando"] = "IDENTIFICARSE";
    mensaje["rol"] = "Ranking";

    QJsonDocument doc(mensaje);
    m_socket->write(doc.toJson(QJsonDocument::Compact) + "\n");

    qDebug() << "Cliente Ranking conectado e identificado.";
    
    m_timer->start(5000);  // Comenzar a solicitar cada 5 segundos
}

void ClienteRanking::onDatosRecibidos() {
    m_buffer.append(m_socket->readAll());

    while (m_buffer.contains('\n')) {
        int pos = m_buffer.indexOf('\n');
        QByteArray linea = m_buffer.left(pos);
        m_buffer.remove(0, pos + 1);

        QJsonDocument doc = QJsonDocument::fromJson(linea);
        if (!doc.isObject()) continue;

        QJsonObject obj = doc.object();
        if (obj["evento"].toString() == "ACTUALIZACION_ESTADO_GENERAL") {
            QJsonObject data = obj["data"].toObject();
            emit rankingActualizado(data["ranking"].toArray());
        }
    }
}

void ClienteRanking::solicitarEstado() {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject mensaje;
        mensaje["comando"] = "SOLICITAR_ESTADO";
        QJsonDocument doc(mensaje);
        m_socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
    }
}

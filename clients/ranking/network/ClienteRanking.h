#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

class ClienteRanking : public QObject {
    Q_OBJECT

public:
    explicit ClienteRanking(QObject* parent = nullptr);
    void conectar(const QString& host, quint16 puerto);

signals:
    void rankingActualizado(const QJsonArray& ranking);

private slots:
    void onConectado();
    void onDatosRecibidos();
    void solicitarEstado();  // NUEVO SLOT

private:
    QTcpSocket* m_socket;
    QByteArray m_buffer;
    QTimer* m_timer;         // NUEVO ATRIBUTO
};

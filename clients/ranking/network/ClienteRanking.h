#ifndef CLIENTERANKING_H
#define CLIENTERANKING_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>

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
    void onError(QAbstractSocket::SocketError socketError);  // ✅ Declaración agregada

private:
    QTcpSocket* m_socket;
    QByteArray m_buffer;
};

#endif // CLIENTERANKING_H

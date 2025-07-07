#pragma once

#include <QTcpSocket>
#include <QObject>

class ClienteRecepcionista : public QObject {
    Q_OBJECT

public:
    explicit ClienteRecepcionista(QObject* parent = nullptr);
    void conectarAlServidor();
    void enviarIdentificacion();
    void enviarNuevoPedido(int mesa, int idRecepcionista, const QJsonArray& platos);

signals:
    void mensajeRecibido(const QJsonObject& json);

private slots:
    void onDatosRecibidos();

private:
    QTcpSocket socket;
};
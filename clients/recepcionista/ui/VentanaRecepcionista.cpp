#include "VentanaRecepcionista.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

VentanaRecepcionista::VentanaRecepcionista(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);

    QPushButton* botonPedido = new QPushButton("Enviar pedido mesa 4", this);
    layout->addWidget(botonPedido);

    connect(botonPedido, &QPushButton::clicked, this, [this]() {
        QJsonArray platos;

        QJsonObject plato1;
        plato1["id"] = 101;
        plato1["cantidad"] = 2;
        platos.append(plato1);

        QJsonObject plato2;
        plato2["id"] = 102;
        plato2["cantidad"] = 1;
        platos.append(plato2);

        cliente.enviarNuevoPedido(4, 101, platos);
    });

    connect(&cliente, &ClienteRecepcionista::mensajeRecibido, this, [](const QJsonObject& msg){
        qDebug() << "📩 Evento recibido:" << msg;
    });

    cliente.conectarAlServidor();
}

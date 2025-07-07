// VentanaRecepcionista.cpp
#include "VentanaRecepcionista.h"
#include <QVBoxLayout>
#include <QSpinBox>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>

VentanaRecepcionista::VentanaRecepcionista(QWidget *parent) : QWidget(parent) {
    cliente.conectarAlServidor("127.0.0.1", 5555);
    cargarUI();
}

void VentanaRecepcionista::cargarUI() {
    comboMesas = new QComboBox(this);
    for (int i = 1; i <= 10; ++i)
        comboMesas->addItem(QString::number(i));

    tablaPlatos = new QTableWidget(0, 2, this);
    tablaPlatos->setHorizontalHeaderLabels({"Plato", "Cantidad"});

    // EJEMPLO: Simular menú con 2 platos
    QStringList nombres = {"Ceviche", "Lomo Saltado"};
    QList<int> ids = {101, 201};

    for (int i = 0; i < nombres.size(); ++i) {
        tablaPlatos->insertRow(i);
        auto *item = new QTableWidgetItem(nombres[i]);
        item->setData(Qt::UserRole, ids[i]);
        tablaPlatos->setItem(i, 0, item);

        auto *spin = new QSpinBox();
        spin->setRange(0, 10);
        tablaPlatos->setCellWidget(i, 1, spin);
    }

    botonEnviar = new QPushButton("Enviar Pedido", this);
    connect(botonEnviar, &QPushButton::clicked, this, [this]() {
        QJsonArray platos;
        for (int i = 0; i < tablaPlatos->rowCount(); ++i) {
            int cantidad = static_cast<QSpinBox *>(tablaPlatos->cellWidget(i, 1))->value();
            int idPlato = tablaPlatos->item(i, 0)->data(Qt::UserRole).toInt();

            if (cantidad > 0) {
                platos.append(QJsonObject{
                    {"id", idPlato},
                    {"cantidad", cantidad}
                });
            }
        }

        if (platos.isEmpty()) {
            QMessageBox::warning(this, "Error", "Debe elegir al menos un plato");
            return;
        }

        int mesa = comboMesas->currentText().toInt();
        cliente.enviarNuevoPedido(mesa, 101, platos);
        QMessageBox::information(this, "Enviado", "Pedido enviado correctamente");
    });

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(comboMesas);
    layout->addWidget(tablaPlatos);
    layout->addWidget(botonEnviar);
}

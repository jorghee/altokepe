#include "VentanaRecepcionista.h"
#include <QVBoxLayout>
#include <QSpinBox>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>

VentanaRecepcionista::VentanaRecepcionista(QWidget *parent) : QWidget(parent) {
    cliente.conectarAlServidor("127.0.0.1", 5555);
    cargarUI();
    connect(&cliente, &ClienteRecepcionista::menuActualizado,
            this, &VentanaRecepcionista::actualizarMenu);
}


void VentanaRecepcionista::cargarUI() {
    comboMesas = new QComboBox(this);
    for (int i = 1; i <= 10; ++i)
        comboMesas->addItem(QString::number(i));

    tablaPlatos = new QTableWidget(0, 2, this);
    tablaPlatos->setHorizontalHeaderLabels({"Plato", "Cantidad"});

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

void VentanaRecepcionista::actualizarMenu(const QJsonArray &menu) {
    tablaPlatos->setRowCount(0);  // Limpiar

    for (int i = 0; i < menu.size(); ++i) {
        QJsonObject plato = menu[i].toObject();
        int row = tablaPlatos->rowCount();
        tablaPlatos->insertRow(row);

        QString nombre = plato["nombre"].toString();
        int id = plato["id"].toInt();

        auto *item = new QTableWidgetItem(nombre);
        item->setData(Qt::UserRole, id);
        tablaPlatos->setItem(row, 0, item);

        auto *spin = new QSpinBox();
        spin->setRange(0, 10);
        tablaPlatos->setCellWidget(row, 1, spin);
    }
}

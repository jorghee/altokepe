#include "PanelPedido.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QMessageBox>
#include <QJsonObject>

PanelPedido::PanelPedido(QWidget *parent)
    : QWidget(parent) {
    cliente.conectarAlServidor("127.0.0.1", 5555);
    configurarUI();

    connect(&cliente, &ClienteRecepcionista::menuActualizado,
            this, &PanelPedido::actualizarMenu);
}

void PanelPedido::configurarUI() {
    auto *layout = new QVBoxLayout(this);

    labelMesa = new QLabel("Mesa: --", this);
    labelPedido = new QLabel("Pedido N°: 0001", this);

    inputNombre = new QLineEdit(this);
    inputNombre->setPlaceholderText("Nombre del cliente");

    auto *tituloPlatos = new QLabel("Platos:", this);
    tituloPlatos->setStyleSheet("font-weight: bold;");

    listaPlatos = new QListWidget(this);
    connect(listaPlatos, &QListWidget::itemClicked, this, [=](QListWidgetItem *item) {
        int id = item->data(Qt::UserRole).toInt();
        QString nombre = item->text();
        agregarPlatoAlPedido(id, nombre);
    });

    tablaPedido = new QTableWidget(0, 3, this);
    tablaPedido->setHorizontalHeaderLabels({"Plato", "Cantidad", "Acción"});
    tablaPedido->horizontalHeader()->setStretchLastSection(true);

    // Ajustar ancho de columnas
    tablaPedido->setColumnWidth(0, 250); // Plato
    tablaPedido->setColumnWidth(1, 100); // Cantidad
    tablaPedido->setColumnWidth(2, 60);  // Acción

    botonEnviar = new QPushButton("Enviar Pedido", this);
    botonEnviar->setObjectName("botonEnviar"); // para aplicar estilo bold por QSS
    botonEnviar->setStyleSheet("font-weight: bold;");
    connect(botonEnviar, &QPushButton::clicked, this, &PanelPedido::enviarPedido);

    layout->addWidget(labelMesa);
    layout->addWidget(labelPedido);
    layout->addWidget(inputNombre);
    layout->addSpacing(10);
    layout->addWidget(tituloPlatos);
    layout->addWidget(listaPlatos);
    layout->addSpacing(10);
    layout->addWidget(tablaPedido);
    layout->addSpacing(10);
    layout->addWidget(botonEnviar);

    setLayout(layout);
}

void PanelPedido::setNumeroMesa(int numero) {
    mesaActual = numero;
    labelMesa->setText(QString("Mesa: %1").arg(numero));
}

void PanelPedido::actualizarMenu(const QJsonArray &menu) {
    listaPlatos->clear();
    menuPlatos.clear();

    for (const QJsonValue &valor : menu) {
        QJsonObject plato = valor.toObject();
        int id = plato["id"].toInt();
        QString nombre = plato["nombre"].toString();

        auto *item = new QListWidgetItem(nombre);
        item->setData(Qt::UserRole, id);
        listaPlatos->addItem(item);

        menuPlatos[id] = nombre;
    }
}

void PanelPedido::agregarPlatoAlPedido(int idPlato, const QString &nombre) {
    for (int i = 0; i < tablaPedido->rowCount(); ++i) {
        if (tablaPedido->item(i, 0)->data(Qt::UserRole).toInt() == idPlato)
            return;
    }

    int row = tablaPedido->rowCount();
    tablaPedido->insertRow(row);

    auto *item = new QTableWidgetItem(nombre);
    item->setData(Qt::UserRole, idPlato);
    tablaPedido->setItem(row, 0, item);

    auto *spin = new QSpinBox();
    spin->setRange(1, 10);
    tablaPedido->setCellWidget(row, 1, spin);

    auto *btnEliminar = new QPushButton("❌", this);
    btnEliminar->setStyleSheet("background-color: white; color: #F44336; border: none;");
    connect(btnEliminar, &QPushButton::clicked, this, [=]() {
        eliminarPlato(row);
    });
    tablaPedido->setCellWidget(row, 2, btnEliminar);
}

void PanelPedido::eliminarPlato(int fila) {
    tablaPedido->removeRow(fila);
}

void PanelPedido::enviarPedido() {
    if (mesaActual == -1) {
        QMessageBox::warning(this, "Error", "Debe seleccionar una mesa.");
        return;
    }

    QString nombreCliente = inputNombre->text().trimmed();
    if (nombreCliente.isEmpty()) {
        QMessageBox::warning(this, "Error", "Ingrese el nombre del cliente.");
        return;
    }

    QJsonArray platos;
    for (int i = 0; i < tablaPedido->rowCount(); ++i) {
        int id = tablaPedido->item(i, 0)->data(Qt::UserRole).toInt();
        int cantidad = static_cast<QSpinBox *>(tablaPedido->cellWidget(i, 1))->value();

        if (cantidad >= 1) {
            platos.append(QJsonObject{
                {"id", id},
                {"cantidad", cantidad}
            });
        }
    }

    if (platos.isEmpty()) {
        QMessageBox::warning(this, "Error", "Debe agregar al menos un plato.");
        return;
    }

    cliente.enviarNuevoPedido(mesaActual, numeroPedido, platos);

    QMessageBox::information(this, "Enviado", "Pedido enviado correctamente.");

    numeroPedido++;
    labelPedido->setText(QString("Pedido N°: %1").arg(QString("%1").arg(numeroPedido, 4, 10, QChar('0'))));

    inputNombre->clear();
    tablaPedido->setRowCount(0);

    // Reset mesa seleccionada
    mesaActual = -1;
    labelMesa->setText("Mesa: --");
}

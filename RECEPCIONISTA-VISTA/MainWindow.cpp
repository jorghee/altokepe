#include "MainWindow.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QSpinBox>
#include <QPushButton>
#include <QMap>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *rootLayout = new QVBoxLayout(centralWidget);

    // Cabecera
    QLabel *titulo = new QLabel("ALTOKEPE - Recepcionista");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setStyleSheet("font-size: 22px; font-weight: bold;");
    rootLayout->addWidget(titulo);

    // Cuerpo
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    QWidget *leftWidget = new QWidget();
    QWidget *rightWidget = new QWidget();
    leftWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightWidget->setFixedWidth(700); // más ancho aún

    // --- MESA SIMULADA COMO CRUZ (4 mesas)
    QVBoxLayout *mesaLayout = new QVBoxLayout(leftWidget);
    for (int i = 0; i < 4; ++i) {
        QGroupBox *mesaBox = new QGroupBox(QString("Mesa %1").arg(i + 1));
        QGridLayout *layoutMesa = new QGridLayout();

        // Sillas cruzadas
        layoutMesa->addWidget(new QPushButton("S"), 0, 1);
        layoutMesa->addWidget(new QPushButton("S"), 1, 0);
        layoutMesa->addWidget(new QLabel("[ M ]"), 1, 1);
        layoutMesa->addWidget(new QPushButton("S"), 1, 2);
        layoutMesa->addWidget(new QPushButton("S"), 2, 1);

        mesaBox->setLayout(layoutMesa);
        mesaLayout->addWidget(mesaBox);
    }

    // --- FORMULARIO DERECHO
    QVBoxLayout *formLayout = new QVBoxLayout(rightWidget);
    pedidoInput = new QLineEdit();
    mesaInput = new QLineEdit();
    personasInput = new QLineEdit();

    comboPlatos = new QComboBox();
    comboPlatos->addItem("Por elegir");
    comboPlatos->addItems({
        "Aalopuri", "Vadapav", "Sugarcane juice",
        "Panipuri", "Frankie", "Sandwich", "Cold coffee"
    });

    // Simulación de CSV: ID y precios
    QMap<QString, int> idPlato = {
        {"Aalopuri", 1}, {"Vadapav", 2}, {"Sugarcane juice", 3},
        {"Panipuri", 4}, {"Frankie", 5}, {"Sandwich", 6}, {"Cold coffee", 7}
    };
    QMap<QString, double> precios = {
        {"Aalopuri", 20}, {"Vadapav", 20}, {"Sugarcane juice", 25},
        {"Panipuri", 20}, {"Frankie", 50}, {"Sandwich", 60}, {"Cold coffee", 40}
    };

    // TABLA: ID | Nombre | Precio | Cantidad | Total | Botón
    tablaPedidos = new QTableWidget(0, 6);
    tablaPedidos->setHorizontalHeaderLabels({"ID", "Nombre", "Precio", "Cantidad", "Total", "Acción"});
    tablaPedidos->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // BOTÓN AÑADIR
    QPushButton *btnAdd = new QPushButton("Añadir pedido");

    // FORMULARIO VISUAL
    formLayout->addWidget(new QLabel("N° Pedido:"));
    formLayout->addWidget(pedidoInput);
    formLayout->addWidget(new QLabel("N° Mesa:"));
    formLayout->addWidget(mesaInput);
    formLayout->addWidget(new QLabel("N° Personas:"));
    formLayout->addWidget(personasInput);
    formLayout->addWidget(new QLabel("Plato:"));
    formLayout->addWidget(comboPlatos);
    formLayout->addWidget(btnAdd);
    formLayout->addWidget(new QLabel("Pedidos:"));
    formLayout->addWidget(tablaPedidos);

    // ENSAMBLADO
    bodyLayout->addWidget(leftWidget, 3);  // 60%
    bodyLayout->addWidget(rightWidget, 2); // 40%
    rootLayout->addLayout(bodyLayout);

    // --- CONECTAR AÑADIR PEDIDO
    connect(btnAdd, &QPushButton::clicked, this, [=]() {
        QString nombre = comboPlatos->currentText();
        if (nombre == "Por elegir") return;

        double precio = precios[nombre];
        int id = idPlato[nombre];

        int row = tablaPedidos->rowCount();
        tablaPedidos->insertRow(row);

        tablaPedidos->setItem(row, 0, new QTableWidgetItem(QString::number(id)));
        tablaPedidos->setItem(row, 1, new QTableWidgetItem(nombre));
        tablaPedidos->setItem(row, 2, new QTableWidgetItem(QString::number(precio, 'f', 2)));

        QSpinBox *cantidadSpin = new QSpinBox();
        cantidadSpin->setRange(1, 20);
        cantidadSpin->setValue(1);
        tablaPedidos->setCellWidget(row, 3, cantidadSpin);

        QTableWidgetItem *totalItem = new QTableWidgetItem(QString::number(precio, 'f', 2));
        totalItem->setFlags(totalItem->flags() & ~Qt::ItemIsEditable);
        tablaPedidos->setItem(row, 4, totalItem);

        QPushButton *btnDel = new QPushButton("Eliminar");
        tablaPedidos->setCellWidget(row, 5, btnDel);

        // Recalcular total cuando cambia la cantidad
        connect(cantidadSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) {
            double nuevoTotal = value * precio;
            totalItem->setText(QString::number(nuevoTotal, 'f', 2));
        });

        // Eliminar fila
        connect(btnDel, &QPushButton::clicked, this, [=]() {
            tablaPedidos->removeRow(row);
        });
    });
}

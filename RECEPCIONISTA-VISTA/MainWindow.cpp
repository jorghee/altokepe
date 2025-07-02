#include "MainWindow.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QSpinBox>
#include <QPushButton>
#include <QMap>
#include <QDebug>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    // Estilo claro minimalista con naranja plano
    qApp->setStyle("Fusion");
    QPalette palette;
    palette.setColor(QPalette::Window, Qt::white);
    palette.setColor(QPalette::WindowText, QColor(50, 30, 10));
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::black);
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::Button, QColor("#FFA726"));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    qApp->setPalette(palette);

    this->setStyleSheet(R"(
        QPushButton {
            background-color: #FFA726;
            color: white;
            border: none;
            padding: 6px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #FB8C00;
        }
        QLineEdit, QComboBox, QSpinBox {
            border: 1px solid #ccc;
            padding: 4px;
            border-radius: 4px;
            background: white;
        }
        QGroupBox {
            border: 1px solid #ccc;
            border-radius: 6px;
            margin-top: 8px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px 0 4px;
        }
        QTableWidget {
            border: 1px solid #ccc;
            background-color: white;
        }
    )");

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *rootLayout = new QVBoxLayout(centralWidget);
    QLabel *titulo = new QLabel("ALTOKEPE - Recepcionista");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setStyleSheet("font-size: 22px; font-weight: bold;");
    rootLayout->addWidget(titulo);

    QHBoxLayout *bodyLayout = new QHBoxLayout();
    QWidget *leftWidget = new QWidget();
    QWidget *rightWidget = new QWidget();
    leftWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightWidget->setFixedWidth(700);

    QVBoxLayout *mesaLayout = new QVBoxLayout(leftWidget);
    for (int i = 0; i < 4; ++i) {
        QGroupBox *mesaBox = new QGroupBox(QString("Mesa %1").arg(i + 1));
        QGridLayout *layoutMesa = new QGridLayout();
        layoutMesa->addWidget(new QPushButton("S"), 0, 1);
        layoutMesa->addWidget(new QPushButton("S"), 1, 0);
        layoutMesa->addWidget(new QLabel("[ M ]"), 1, 1);
        layoutMesa->addWidget(new QPushButton("S"), 1, 2);
        layoutMesa->addWidget(new QPushButton("S"), 2, 1);
        mesaBox->setLayout(layoutMesa);
        mesaLayout->addWidget(mesaBox);
    }

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

    QMap<QString, int> idPlato = {
        {"Aalopuri", 1}, {"Vadapav", 2}, {"Sugarcane juice", 3},
        {"Panipuri", 4}, {"Frankie", 5}, {"Sandwich", 6}, {"Cold coffee", 7}
    };
    QMap<QString, double> precios = {
        {"Aalopuri", 20}, {"Vadapav", 20}, {"Sugarcane juice", 25},
        {"Panipuri", 20}, {"Frankie", 50}, {"Sandwich", 60}, {"Cold coffee", 40}
    };

    tablaPedidos = new QTableWidget(0, 6);
    tablaPedidos->setHorizontalHeaderLabels({"ID Plato", "Nombre", "Precio", "Cantidad", "Total", "Acción"});
    tablaPedidos->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QPushButton *btnAddPlato = new QPushButton("Agregar plato");
    QPushButton *btnEnviarPedido = new QPushButton("Enviar pedido");

    labelTotal = new QLabel("Total del pedido: S/. 0.00");
    labelTotal->setAlignment(Qt::AlignRight);
    labelTotal->setStyleSheet("font-weight: bold; font-size: 16px;");

    formLayout->addWidget(new QLabel("N° Pedido:"));
    formLayout->addWidget(pedidoInput);
    formLayout->addWidget(new QLabel("N° Mesa:"));
    formLayout->addWidget(mesaInput);
    formLayout->addWidget(new QLabel("N° Personas:"));
    formLayout->addWidget(personasInput);
    formLayout->addWidget(new QLabel("Plato:"));
    formLayout->addWidget(comboPlatos);
    formLayout->addWidget(btnAddPlato);
    formLayout->addSpacing(10);
    formLayout->addWidget(new QLabel("Pedidos:"));
    formLayout->addWidget(tablaPedidos);
    formLayout->addWidget(labelTotal);
    formLayout->addSpacing(10);
    formLayout->addWidget(btnEnviarPedido);

    bodyLayout->addWidget(leftWidget, 3);
    bodyLayout->addWidget(rightWidget, 2);
    rootLayout->addLayout(bodyLayout);

    auto actualizarTotalGeneral = [=]() {
        double total = 0;
        for (int i = 0; i < tablaPedidos->rowCount(); ++i) {
            total += tablaPedidos->item(i, 4)->text().toDouble();
        }
        labelTotal->setText(QString("Total del pedido: S/. %1").arg(total, 0, 'f', 2));
    };

    connect(btnAddPlato, &QPushButton::clicked, this, [=]() {
        QString nombre = comboPlatos->currentText();
        if (nombre == "Por elegir") return;

        double precio = precios[nombre];
        int id = idPlato[nombre];
        int row = tablaPedidos->rowCount();
        tablaPedidos->insertRow(row);

        auto setItem = [&](int col, const QString& text) {
            QTableWidgetItem *item = new QTableWidgetItem(text);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            tablaPedidos->setItem(row, col, item);
        };

        setItem(0, QString::number(id));
        setItem(1, nombre);
        setItem(2, QString::number(precio, 'f', 2));

        QSpinBox *cantidadSpin = new QSpinBox();
        cantidadSpin->setRange(1, 20);
        cantidadSpin->setValue(1);
        tablaPedidos->setCellWidget(row, 3, cantidadSpin);

        QTableWidgetItem *totalItem = new QTableWidgetItem(QString::number(precio, 'f', 2));
        totalItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tablaPedidos->setItem(row, 4, totalItem);

        QPushButton *btnDel = new QPushButton("Eliminar");
        tablaPedidos->setCellWidget(row, 5, btnDel);

        connect(cantidadSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) {
            totalItem->setText(QString::number(value * precio, 'f', 2));
            actualizarTotalGeneral();
        });

        connect(btnDel, &QPushButton::clicked, this, [=]() {
            tablaPedidos->removeRow(row);
            actualizarTotalGeneral();
        });

        actualizarTotalGeneral();
    });

    connect(btnEnviarPedido, &QPushButton::clicked, this, [=]() {
        qDebug() << "=== Pedido enviado ===";
        qDebug() << "N° Pedido:" << pedidoInput->text();
        qDebug() << "Mesa:" << mesaInput->text();
        qDebug() << "Personas:" << personasInput->text();
        for (int i = 0; i < tablaPedidos->rowCount(); ++i) {
            QString nombre = tablaPedidos->item(i, 1)->text();
            int cantidad = qobject_cast<QSpinBox *>(tablaPedidos->cellWidget(i, 3))->value();
            double total = tablaPedidos->item(i, 4)->text().toDouble();
            qDebug() << nombre << "- Cantidad:" << cantidad << "- Total:" << total;
        }
    });
}

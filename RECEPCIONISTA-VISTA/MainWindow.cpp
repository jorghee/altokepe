#include "MainWindow.h"
#include <QHeaderView>
#include <cmath>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), contadorPedidos(1), mesaActual(-1)
{

    for (int i = 0; i < NUMERO_MESAS; ++i) {
        estadoMesas[i] = false;
    }

    setupUI();
    connectSignals();
    aplicarEstilos();
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    setupMesasSection();

    setupPedidoSection();
    cargarMenu();

    QFrame *separador = new QFrame();
    separador->setFrameShape(QFrame::VLine);
    separador->setFrameShadow(QFrame::Sunken);
    separador->setObjectName("separadorVertical");

    mainLayout->addWidget(mesasWidget, 2);
    mainLayout->addWidget(separador);
    mainLayout->addWidget(pedidoWidget, 3);
}

void MainWindow::setupMesasSection()
{
    mesasWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(mesasWidget);

    // Título
    QLabel *titulo = new QLabel("MESAS DEL RESTAURANTE");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setObjectName("tituloSeccion");
    layout->addWidget(titulo);

    QFrame *separadorH = new QFrame();
    separadorH->setFrameShape(QFrame::HLine);
    separadorH->setFrameShadow(QFrame::Sunken);
    separadorH->setObjectName("separadorHorizontal");
    layout->addWidget(separadorH);

    // Grid para las mesas
    mesasLayout = new QGridLayout();
    mesasLayout->setSpacing(20);

    crearBotonesMesas();

    layout->addLayout(mesasLayout);
    layout->addStretch();
}

void MainWindow::setupPedidoSection()
{
    pedidoWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(pedidoWidget);

    // Título
    QLabel *titulo = new QLabel("PEDIDOS");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setObjectName("tituloSeccion");
    layout->addWidget(titulo);

    QFrame *separadorH = new QFrame();
    separadorH->setFrameShape(QFrame::HLine);
    separadorH->setFrameShadow(QFrame::Sunken);
    separadorH->setObjectName("separadorHorizontal");
    layout->addWidget(separadorH);

    pedidoLabel = new QLabel(QString("Pedido #%1").arg(contadorPedidos));
    pedidoLabel->setObjectName("pedidoInfo");
    layout->addWidget(pedidoLabel);

    mesaSeleccionadaLabel = new QLabel("Mesa: Ninguna seleccionada");
    mesaSeleccionadaLabel->setObjectName("mesaInfo");
    layout->addWidget(mesaSeleccionadaLabel);

    // Selector de platos
    QLabel *labelPlatos = new QLabel("Seleccionar Plato:");
    labelPlatos->setObjectName("labelGeneral");
    layout->addWidget(labelPlatos);

    comboPlatos = new QComboBox();
    layout->addWidget(comboPlatos);

    // Campo de descripción
    QLabel *labelDescripcion = new QLabel("Descripción (opcional):");
    labelDescripcion->setObjectName("labelGeneral");
    layout->addWidget(labelDescripcion);

    descripcionEdit = new QLineEdit();
    descripcionEdit->setPlaceholderText("Ej: Sin cebolla, extra picante...");
    layout->addWidget(descripcionEdit);

    // Botón agregar plato
    btnAgregarPlato = new QPushButton("Agregar Plato");
    btnAgregarPlato->setEnabled(false);
    layout->addWidget(btnAgregarPlato);

    // Tabla de pedidos
    tablaPedidos = new QTableWidget(0, 6);
    tablaPedidos->setHorizontalHeaderLabels({"Plato", "Precio", "Cantidad", "Total", "Descripción", "Acción"});

    // Configurar anchos de columnas
    tablaPedidos->setColumnWidth(0, 150);  // Plato
    tablaPedidos->setColumnWidth(1, 80);   // Precio
    tablaPedidos->setColumnWidth(2, 80);   // Cantidad
    tablaPedidos->setColumnWidth(3, 80);   // Total
    tablaPedidos->setColumnWidth(4, 200);  // Descripción más larga
    tablaPedidos->setColumnWidth(5, 80);   // Acción

    tablaPedidos->horizontalHeader()->setStretchLastSection(false);
    layout->addWidget(tablaPedidos);

    // Total
    labelTotal = new QLabel("Total: S/. 0.00");
    labelTotal->setObjectName("totalLabel");
    layout->addWidget(labelTotal);

    // Botones de acción
    QHBoxLayout *botonesLayout = new QHBoxLayout();
    btnNuevoPedido = new QPushButton("Nuevo Pedido");
    btnEnviarPedido = new QPushButton("Enviar Pedido");
    btnEnviarPedido->setEnabled(false);

    botonesLayout->addWidget(btnNuevoPedido);
    botonesLayout->addWidget(btnEnviarPedido);
    layout->addLayout(botonesLayout);
}

void MainWindow::crearBotonesMesas()
{
    botonesMesas.clear();

    int columnas = 3;

    for (int i = 0; i < NUMERO_MESAS; ++i) {
        QPushButton *botonMesa = new QPushButton(QString("Mesa\n\n%1").arg(i + 1));
        botonMesa->setObjectName("mesaButton");
        botonMesa->setFixedSize(200, 200);
        botonMesa->setProperty("numeroMesa", i);
        botonMesa->setProperty("ocupada", false);

        // Conectar señal
        connect(botonMesa, &QPushButton::clicked, this, [=]() {
            mesaClicked(i);
        });

        botonesMesas.append(botonMesa);

        int fila = i / columnas;
        int columna = i % columnas;
        mesasLayout->addWidget(botonMesa, fila, columna);
    }

    for (int i = 0; i < NUMERO_MESAS; ++i) {
        actualizarEstiloMesa(i, false);
    }
}

void MainWindow::connectSignals()
{
    connect(btnAgregarPlato, &QPushButton::clicked, this, &MainWindow::agregarPlato);
    connect(btnEnviarPedido, &QPushButton::clicked, this, &MainWindow::enviarPedido);
    connect(btnNuevoPedido, &QPushButton::clicked, this, &MainWindow::nuevoPedido);

}

void MainWindow::mesaClicked(int numeroMesa)
{
    if (estadoMesas[numeroMesa]) {
        if (verificarDesocuparMesa(numeroMesa)) {
            estadoMesas[numeroMesa] = false;
            actualizarEstiloMesa(numeroMesa, false);

            if (mesaActual == numeroMesa) {
                mesaActual = -1;
                mesaSeleccionadaLabel->setText("Mesa: Ninguna seleccionada");
                btnAgregarPlato->setEnabled(false);
                btnEnviarPedido->setEnabled(false);
                limpiarFormulario();
            }

            qDebug() << "Mesa" << (numeroMesa + 1) << "desocupada";
        }
    } else {
        seleccionarMesa(numeroMesa);
    }
}

void MainWindow::seleccionarMesa(int numeroMesa)
{
    mesaActual = numeroMesa;
    mesaSeleccionadaLabel->setText(QString("Mesa: %1").arg(numeroMesa + 1));
    btnAgregarPlato->setEnabled(true);

    for (int i = 0; i < NUMERO_MESAS; ++i) {
        if (i == numeroMesa) {
            botonesMesas[i]->setProperty("seleccionada", true);
        } else {
            botonesMesas[i]->setProperty("seleccionada", false);
        }
        botonesMesas[i]->style()->unpolish(botonesMesas[i]);
        botonesMesas[i]->style()->polish(botonesMesas[i]);
    }

    qDebug() << "Mesa" << (numeroMesa + 1) << "seleccionada";
}

void MainWindow::agregarPlato()
{
    QString nombrePlato = comboPlatos->currentText();
    if (nombrePlato == "Seleccione un plato..." || mesaActual == -1) return;

    if (!mapaPlatos.contains(nombrePlato)) return;

    Plato plato = mapaPlatos[nombrePlato];
    double precio = plato.precio;
    QString descripcion = descripcionEdit->text().trimmed();

    int fila = tablaPedidos->rowCount();
    tablaPedidos->insertRow(fila);

    tablaPedidos->setItem(fila, 0, new QTableWidgetItem(nombrePlato));
    tablaPedidos->setItem(fila, 1, new QTableWidgetItem(QString::number(precio, 'f', 2)));

    QSpinBox *spinCantidad = new QSpinBox();
    spinCantidad->setRange(1, 20);
    spinCantidad->setValue(1);
    spinCantidad->setFixedHeight(30);
    spinCantidad->setObjectName("spinCantidad");
    tablaPedidos->setCellWidget(fila, 2, spinCantidad);

    QTableWidgetItem *totalItem = new QTableWidgetItem(QString::number(precio, 'f', 2));
    tablaPedidos->setItem(fila, 3, totalItem);

    QTableWidgetItem *descripcionItem = new QTableWidgetItem(descripcion.isEmpty() ? "-" : descripcion);
    tablaPedidos->setItem(fila, 4, descripcionItem);

    QPushButton *btnEliminar = new QPushButton("Eliminar");
    btnEliminar->setObjectName("btnEliminar");
    btnEliminar->setFixedWidth(75);
    tablaPedidos->setCellWidget(fila, 5, btnEliminar);

    connect(spinCantidad, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int cantidad) {
        double total = cantidad * precio;
        totalItem->setText(QString::number(total, 'f', 2));
        actualizarTotalGeneral();
    });

    connect(btnEliminar, &QPushButton::clicked, this, [=]() {
        int filaEliminar = tablaPedidos->indexAt(btnEliminar->pos()).row();
        tablaPedidos->removeRow(filaEliminar);
        actualizarTotalGeneral();

        if (tablaPedidos->rowCount() == 0) {
            btnEnviarPedido->setEnabled(false);
        }
    });

    actualizarTotalGeneral();
    btnEnviarPedido->setEnabled(true);

    comboPlatos->setCurrentIndex(0);
    descripcionEdit->clear();
}


void MainWindow::enviarPedido()
{
    if (mesaActual == -1 || tablaPedidos->rowCount() == 0) return;

    // Marcar mesa como ocupada
    estadoMesas[mesaActual] = true;
    actualizarEstiloMesa(mesaActual, true);

    // Log del pedido
    qDebug() << "PEDIDO ENVIADO ";
    qDebug() << "Pedido #" << contadorPedidos;
    qDebug() << "Mesa:" << (mesaActual + 1);

    for (int i = 0; i < tablaPedidos->rowCount(); ++i) {
        QString plato = tablaPedidos->item(i, 0)->text();
        QSpinBox *spin = qobject_cast<QSpinBox*>(tablaPedidos->cellWidget(i, 2));
        double total = tablaPedidos->item(i, 3)->text().toDouble();
        QString descripcion = tablaPedidos->item(i, 4)->text();

        if (spin) {
            qDebug() << "  -" << plato << "x" << spin->value() << "= S/." << total;
            if (descripcion != "-") {
                qDebug() << "    Descripción:" << descripcion;
            }
        }
    }

    QString totalGeneral = labelTotal->text();
    qDebug() << totalGeneral;
    qDebug() << "======================";

    // Limpiar formulario y preparar para el siguiente pedido
    contadorPedidos++;
    limpiarFormulario();

    // Resetear selección de mesa
    mesaActual = -1;
    mesaSeleccionadaLabel->setText("Mesa: Ninguna seleccionada");
    btnAgregarPlato->setEnabled(false);
    btnEnviarPedido->setEnabled(false);

    // Actualizar estilos para quitar selección
    for (QPushButton *btn : botonesMesas) {
        btn->setProperty("seleccionada", false);
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}

void MainWindow::nuevoPedido()
{
    limpiarFormulario();

    // Resetear selección de mesa
    mesaActual = -1;
    mesaSeleccionadaLabel->setText("Mesa: Ninguna seleccionada");
    btnAgregarPlato->setEnabled(false);
    btnEnviarPedido->setEnabled(false);

    // Actualizar estilos para quitar selección
    for (QPushButton *btn : botonesMesas) {
        btn->setProperty("seleccionada", false);
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}

void MainWindow::actualizarTotalGeneral()
{
    double total = 0.0;
    for (int i = 0; i < tablaPedidos->rowCount(); ++i) {
        if (tablaPedidos->item(i, 3)) {
            total += tablaPedidos->item(i, 3)->text().toDouble();
        }
    }
    labelTotal->setText(QString("Total: S/. %1").arg(total, 0, 'f', 2));
}

void MainWindow::limpiarFormulario()
{
    pedidoLabel->setText(QString("Pedido #%1").arg(contadorPedidos));
    tablaPedidos->setRowCount(0);
    labelTotal->setText("Total: S/. 0.00");
    comboPlatos->setCurrentIndex(0);
    descripcionEdit->clear();
}

bool MainWindow::verificarDesocuparMesa(int numeroMesa)
{
    QMessageBox::StandardButton respuesta = QMessageBox::question(
        this,
        "Confirmar acción",
        QString("¿Está seguro de que desea desocupar la Mesa %1?").arg(numeroMesa + 1),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    return respuesta == QMessageBox::Yes;
}

void MainWindow::actualizarEstiloMesa(int numeroMesa, bool ocupada)
{
    if (numeroMesa >= 0 && numeroMesa < botonesMesas.size()) {
        botonesMesas[numeroMesa]->setProperty("ocupada", ocupada);
        botonesMesas[numeroMesa]->style()->unpolish(botonesMesas[numeroMesa]);
        botonesMesas[numeroMesa]->style()->polish(botonesMesas[numeroMesa]);
    }
}

void MainWindow::cargarMenu()
{
    QFile archivo("../../../DATA/Menu.csv");
    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "No se pudo abrir el archivo";
        return;
    }

    QTextStream in(&archivo);
    QString header = in.readLine(); // Leer encabezado

    while (!in.atEnd()) {
        QString linea = in.readLine();
        QStringList datos = linea.split(',');

        if (datos.size() != 5) continue;

        Plato plato;
        plato.nombre = datos[1];
        plato.precio = datos[2].toDouble();
        plato.tiempoPreparacion = datos[3].toInt();
        plato.categoria = datos[4];

        mapaPlatos[plato.nombre] = plato;
    }

    archivo.close();

    // Llenar el comboBox
    comboPlatos->addItem("Seleccione un plato...");
    for (const auto &plato : mapaPlatos.keys()) {
        comboPlatos->addItem(plato);
    }
}

void MainWindow::aplicarEstilos()
{
    // Colores planos definidos como variables
    QString colorFondo = "#ffffff";
    QString colorPrincipal = "#ECAC5B";
    QString colorTexto = "#333333";
    QString colorTextoClaro = "#ffffff";
    QString colorResaltado = "#0066cc";
    QString colorAdvertencia = "#ff3333";
    QString colorGrisClaro = "#f5f5f5";
    QString colorSeparador = "#d0d0d0";

    setStyleSheet(QString(R"(
        QMainWindow {
            background-color: %1;
        }

        #tituloSeccion {
            font-size: 24px;
            font-weight: bold;
            color: %3;
            padding: 15px;
            margin-bottom: 5px;
        }

        #separadorVertical {
            color: %8;
            background-color: %8;
            border: 1px solid %8;
            max-width: 2px;
        }

        #separadorHorizontal {
            color: %8;
            background-color: %8;
            border: 1px solid %8;
            max-height: 2px;
        }

        #mesaButton {
            background-color: %2;
            color: %4;
            font-weight: bold;
            font-size: 18px;
            text-align: center;
            margin-top: 40px;
        }

        #mesaButton:hover {
            background-color: %5;
        }

        #mesaButton[ocupada="true"] {
            background-color: %6;
        }

        #mesaButton[seleccionada="true"] {
            background-color: %5;
            border: 3px solid %3;
        }

        #pedidoInfo, #mesaInfo {
            font-weight: bold;
            color: %4;
            background-color: %2;
            padding: 10px;
            border-radius: 8px;
            font-size: 14px;
            margin: 5px 0;
        }

        #labelGeneral {
            font-weight: bold;
            color: %3;
            font-size: 14px;
            margin-top: 10px;
        }

        #totalLabel {
            font-size: 20px;
            font-weight: bold;
            color: %4;
            background-color: %2;
            padding: 15px;
            border-radius: 8px;
            margin: 10px 0;
        }

        QPushButton {
            background-color: %2;
            color: %4;
            padding: 10px 20px;
            border-radius: 8px;
            font-weight: bold;
            font-size: 14px;
            border: none;
            min-height: 20px;
        }

        QPushButton:hover {
            background-color: %5;
        }

        QPushButton:disabled {
            background-color: %7;
            color: #999999;
        }

        #btnEliminar {
            background-color: %6;
            color: %4;
            font-size: 12px;
            padding: 5px 10px;
            border-radius: 5px;
            min-width: 65px;
            max-width: 75px;
        }

        #btnEliminar:hover {
            background-color: #cc0000;
        }

        QComboBox, QLineEdit {
            padding: 8px;
            border: 2px solid %7;
            border-radius: 6px;
            background-color: %1;
            color: %3;
            font-size: 14px;
            min-height: 25px;
        }

        QComboBox:focus, QLineEdit:focus {
            border: 2px solid %2;
        }

        #spinCantidad {
            padding: 4px;
            border: 2px solid %7;
            border-radius: 6px;
            background-color: %1;
            color: %3;
            font-size: 14px;
            font-weight: bold;
        }

        #spinCantidad:focus {
            border: 2px solid %2;
        }

        #spinCantidad::up-button, #spinCantidad::down-button {
            width: 20px;
            height: 14px;
            background-color: %2;
            border: 1px solid %2;
        }

        #spinCantidad::up-button:hover, #spinCantidad::down-button:hover {
            background-color: %5;
        }

        #spinCantidad::up-arrow {
            image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTAiIGhlaWdodD0iMTAiIHZpZXdCb3g9IjAgMCAxMCAxMCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTUgMkw4IDZIMloiIGZpbGw9IndoaXRlIi8+Cjwvc3ZnPgo=);
        }

        #spinCantidad::down-arrow {
            image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTAiIGhlaWdodD0iMTAiIHZpZXdCb3g9IjAgMCAxMCAxMCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTUgOEwyIDRIOFoiIGZpbGw9IndoaXRlIi8+Cjwvc3ZnPgo=);
        }

        QTableWidget {
            background-color: %1;
            border: 1px solid %7;
            border-radius: 6px;
            gridline-color: %7;
        }

        QTableWidget::item {
            padding: 8px;
            color: %3;
            border-bottom: 1px solid %7;
        }

        QTableWidget::item:selected {
            background-color: %7;
        }

        QHeaderView::section {
            background-color: %2;
            padding: 12px;
            font-weight: bold;
            color: %4;
            border: none;
            border-right: 1px solid %4;
        }

        QHeaderView::section:last {
            border-right: none;
        }
    )")
                      .arg(colorFondo)           // %1
                      .arg(colorPrincipal)       // %2
                      .arg(colorTexto)           // %3
                      .arg(colorTextoClaro)      // %4
                      .arg(colorResaltado)       // %5
                      .arg(colorAdvertencia)     // %6
                      .arg(colorGrisClaro)       // %7
                      .arg(colorSeparador));     // %8
}

#include "VentanaRanking.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QFont>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <algorithm>

//cambio
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QHBoxLayout>

VentanaRanking::VentanaRanking(QWidget* parent) : QWidget(parent) {
    setWindowTitle("Ranking de Platos Vendidos");
    resize(800, 600); // cambio
    this->setStyleSheet("background-color: white;");

    
    tablaRanking = new QTableWidget(this);
    tablaRanking->setColumnCount(4);
    tablaRanking->setHorizontalHeaderLabels({ "Puesto", "Nombre", "Unidades", "Precio" });
    tablaRanking->setColumnWidth(3, 100); // Fija el ancho de la columna "Precio" (columna 3)
    tablaRanking->horizontalHeader()->setStretchLastSection(true);
    tablaRanking->verticalHeader()->setVisible(false);

    tablaRanking->setStyleSheet(R"(
        QHeaderView::section {
            background-color: #ff9900;
            color: white;
            font-weight: bold;
            padding: 6px;
        }
        QTableWidget {
            font-size: 14px;
        }
    )");
    
    // Título
    QLabel* tituloMenu = new QLabel("Carta Menú");
    tituloMenu->setAlignment(Qt::AlignCenter);
    tituloMenu->setStyleSheet("font-size: 40px; font-weight: bold; color: #e65100;");
    tituloMenu->setContentsMargins(0, 0, 0, 0);

    // Frase decorativa
    QLabel* fraseMenu = new QLabel("No dejes para mañana lo que puedes comer hoy");
    fraseMenu->setAlignment(Qt::AlignCenter);
    fraseMenu->setStyleSheet("font-size: 22px; font-style: italic; color: #6d4c41;");

    //cambio
    scrollMenu = new QScrollArea(this);
    menuContainer = new QWidget();
    gridMenuLayout = new QGridLayout(menuContainer);
    menuContainer->setLayout(gridMenuLayout);
    scrollMenu->setWidget(menuContainer);
    scrollMenu->setWidgetResizable(true);

    //Cambiooo
    QVBoxLayout* menuLayout = new QVBoxLayout();
    menuLayout->setContentsMargins(0, 0, 0, 0); // eliminar margen superior
    menuLayout->setSpacing(10);
    menuLayout->addWidget(tituloMenu);
    menuLayout->addWidget(fraseMenu);
    menuLayout->addWidget(scrollMenu);

    QWidget* menuWidget = new QWidget();
    menuWidget->setLayout(menuLayout);

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(tablaRanking, 3);
    layout->addWidget(menuWidget, 7);
    setLayout(layout);

    //cambio
    QFile file("../../../data/menu.json");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        m_menu = doc.array();
        mostrarMenuAgrupado(m_menu);  // Llama a la función para mostrar
    } else {
        qWarning() << "No se pudo abrir el archivo del menú.";
    }
}

void VentanaRanking::actualizarRanking(const QJsonArray& ranking) {
    tablaRanking->setRowCount(0);

    QVector<QPair<QString, int>> datos;
    QMap<QString, double> precios;

    int maxVentas = 0;

    for (const QJsonValue& val : ranking) {
        QJsonObject obj = val.toObject();
        QString nombre = obj["nombre"].toString();
        int unidades = obj["cantidad"].toInt();
        double precio = obj["precio"].toDouble();

        datos.append({nombre, unidades});
        precios[nombre] = precio;

        if (unidades > maxVentas)
            maxVentas = unidades;
    }

    std::sort(datos.begin(), datos.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    tablaRanking->setRowCount(datos.size());

    for (int i = 0; i < datos.size(); ++i) {
        const QString& nombre = datos[i].first;
        int unidades = datos[i].second;
        double precio = precios[nombre];

        // Columna 0: Puesto
        QTableWidgetItem* puestoItem = new QTableWidgetItem(QString::number(i + 1));
        if (i == 0) {
            puestoItem->setBackground(QColor("#ffff66"));  // amarillo para el 1er puesto
            puestoItem->setFont(QFont("Arial", 10, QFont::Bold));
        }
        tablaRanking->setItem(i, 0, puestoItem);

        // Columna 1: Nombre
        QTableWidgetItem* nombreItem = new QTableWidgetItem(nombre);
        tablaRanking->setItem(i, 1, nombreItem);

        // Columna 2: Unidades
        QTableWidgetItem* unidadesItem = new QTableWidgetItem(QString::number(unidades));
        tablaRanking->setItem(i, 2, unidadesItem);

        // Columna 3: Precio
        QTableWidgetItem* precioItem = new QTableWidgetItem(QString("S/. %1").arg(precio, 0, 'f', 2));
        tablaRanking->setItem(i, 3, precioItem);
    }
}

//cambio
void VentanaRanking::mostrarMenuAgrupado(const QJsonArray& menu) {
    QMap<QString, QList<QPair<QString, double>>> platosPorEstacion;

    for (const QJsonValue& val : menu) {
        QJsonObject obj = val.toObject();
        QString estacion = obj["estacion"].toString();
        QString nombre = obj["nombre"].toString();
        double costo = obj["costo"].toDouble();

        platosPorEstacion[estacion].append(qMakePair(nombre, costo));
    }

    //  Limpiar layout anterior
    QLayoutItem* item;
    while ((item = gridMenuLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    //  Mostrar en 2 columnas
    int col = 0;
    int row = 0;
    const int colCount = 2;

    for (auto it = platosPorEstacion.begin(); it != platosPorEstacion.end(); ++it) {
        QString categoria = it.key();
        QList<QPair<QString, double>> platos = it.value();

        QGroupBox* grupo = new QGroupBox(categoria);
        QVBoxLayout* vbox = new QVBoxLayout();

        for (const auto& par : platos) {
            QWidget* filaWidget = new QWidget();
            filaWidget->setObjectName("itemPlato");
            QHBoxLayout* filaLayout = new QHBoxLayout(filaWidget);
            filaLayout->setContentsMargins(0, 0, 0, 0);  // elimina márgenes
            filaLayout->setSpacing(0);                  // elimina espacio entre widgets

            QLabel* nombreLabel = new QLabel(par.first);
            QLabel* precioLabel = new QLabel(QString("S/ %1").arg(par.second, 0, 'f', 2));

            nombreLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            precioLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

            nombreLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            precioLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

            // Estilos sin borde ni fondo
            nombreLabel->setStyleSheet("font-size: 15px; color: #333; font-weight: bold; border-radius: 6px; background:rgba(226, 225, 225, 0.47);");
            precioLabel->setStyleSheet("font-size: 15px; color: #ff6600; font-weight: bold; border-radius: 6px; background:rgba(226, 225, 225, 0.47);");
            filaLayout->addWidget(nombreLabel);
            filaLayout->addWidget(precioLabel);
            vbox->addWidget(filaWidget);


        }

        grupo->setLayout(vbox);
        gridMenuLayout->addWidget(grupo, row, col);

        col++;
        if (col >= colCount) {
            col = 0;
            row++;
        }
    }
}

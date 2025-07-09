#include "VentanaRanking.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QFont>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <algorithm>

VentanaRanking::VentanaRanking(QWidget* parent) : QWidget(parent) {
    this->setStyleSheet("background-color: white;");

    QVBoxLayout* layout = new QVBoxLayout(this);
    tablaRanking = new QTableWidget(this);
    tablaRanking->setColumnCount(4);
    tablaRanking->setHorizontalHeaderLabels({ "Puesto", "Nombre", "Unidades", "Precio" });
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

    layout->addWidget(tablaRanking);
    setLayout(layout);
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

#include "PanelMesas.h"
#include <QPushButton>
#include <QGridLayout>

PanelMesas::PanelMesas(QWidget *parent)
    : QWidget(parent) {
    layout = new QGridLayout(this);
    layout->setHorizontalSpacing(30);
    layout->setVerticalSpacing(30);

    crearBotones();
}

void PanelMesas::crearBotones() {
    const int totalMesas = 9;
    const int columnas = 3;

    for (int i = 0; i < totalMesas; ++i) {
        int numeroMesa = i + 1;
        QPushButton *boton = new QPushButton(QString("Mesa %1").arg(numeroMesa), this);
        boton->setObjectName("mesa");
        boton->setCheckable(true);
        boton->setMinimumSize(300, 250);
        boton->setMaximumSize(300, 250);
        botonesMesa.append(boton);
        estadosMesa.append(false);  // libre por defecto

        int fila = i / columnas;
        int columna = i % columnas;
        layout->addWidget(boton, fila, columna);

        connect(boton, &QPushButton::clicked, this, [this, i, numeroMesa]() {
            estadosMesa[i] = !estadosMesa[i];  // cambiar estado
            actualizarEstilos();

            if (estadosMesa[i]) {
                emit mesaSeleccionada(numeroMesa);  // solo si está ocupada
            }
        });
    }

    actualizarEstilos(); // establecer color inicial
}

void PanelMesas::actualizarEstilos() {
    for (int i = 0; i < botonesMesa.size(); ++i) {
        QPushButton *boton = botonesMesa[i];

        if (estadosMesa[i]) {
            boton->setStyleSheet("background-color: #ff5c5c; color: white; font-weight: bold;");
        } else {
            boton->setStyleSheet("");  // por defecto (gris del tema)
        }
    }
}

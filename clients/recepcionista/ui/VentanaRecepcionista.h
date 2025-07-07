#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include "../network/ClienteRecepcionista.h"

class VentanaRecepcionista : public QWidget {
    Q_OBJECT
public:
    VentanaRecepcionista(QWidget *parent = nullptr);

private:
    QComboBox *comboMesas;
    QTableWidget *tablaPlatos;
    QPushButton *botonEnviar;
    ClienteRecepcionista cliente;

    void cargarUI();
    
public slots:
    void actualizarMenu(const QJsonArray &menuRecibido);
};

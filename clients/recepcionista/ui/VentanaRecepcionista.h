#pragma once

#include <QWidget>
#include "../network/ClienteRecepcionista.h"

class VentanaRecepcionista : public QWidget {
    Q_OBJECT

public:
    VentanaRecepcionista(QWidget* parent = nullptr);

private:
    ClienteRecepcionista cliente;
};
#include <QApplication>
#include "ui/VentanaEstacionesUnificadas.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Estaciones de Cocina");

    VentanaEstacionesUnificadas principal;
    principal.show();

    return app.exec();
}

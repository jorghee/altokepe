#include <QApplication>
#include "ui/VentanaRecepcionista.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Recepcionista Altokepe");

    VentanaRecepcionista ventana;
    ventana.show();

    return app.exec();
}
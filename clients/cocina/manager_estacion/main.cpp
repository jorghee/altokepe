#include <QApplication>
#include "network/ClienteEstacionApp.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Estación de Cocina");

    // Puedes cambiar el nombreEstacion por "Carnes", "Postres", etc.
    ClienteEstacionApp clienteEstacion("Carnes");
    clienteEstacion.iniciar();

    return app.exec();
}

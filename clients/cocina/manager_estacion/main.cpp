#include <QApplication>
#include "network/ClienteEstacionApp.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Estaciones de Cocina");

    ClienteEstacionApp* estacionCarnes = new ClienteEstacionApp("Carnes");
    ClienteEstacionApp* estacionFrios = new ClienteEstacionApp("Fríos");
    ClienteEstacionApp* estacionPostres = new ClienteEstacionApp("Postres");
    ClienteEstacionApp* estacionGuisos = new ClienteEstacionApp("Guisos");
    ClienteEstacionApp* estacionBebidas = new ClienteEstacionApp("Bebidas");
    estacionCarnes->iniciar();
    estacionFrios->iniciar();
    estacionPostres->iniciar();
    estacionGuisos->iniciar();
    estacionBebidas->iniciar();
    return app.exec();
}

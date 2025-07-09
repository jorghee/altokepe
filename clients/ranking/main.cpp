#include <QApplication>
#include "ui/VentanaRanking.h"
#include "network/ClienteRanking.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    VentanaRanking ventana;
    ClienteRanking cliente;

    QObject::connect(&cliente, &ClienteRanking::rankingActualizado, &ventana, &VentanaRanking::actualizarRanking);

    cliente.conectar("127.0.0.1", 5555);
    ventana.show();

    return app.exec();
}

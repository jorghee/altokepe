#include <QApplication>
#include "ui/VentanaRanking.h"
#include "network/ClienteRanking.h"

#include <QFile>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    VentanaRanking ventana;
    ClienteRanking cliente;

    QObject::connect(&cliente, &ClienteRanking::rankingActualizado, &ventana, &VentanaRanking::actualizarRanking);

    // camnbio
    QFile styleFile(":/styles.qss");  // Asegúrate de que 'styles.qss' está listado en resources.qrc
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = styleFile.readAll();
        app.setStyleSheet(styleSheet);
    }

    cliente.conectar("127.0.0.1", 5555);
    ventana.show();

    return app.exec();
}

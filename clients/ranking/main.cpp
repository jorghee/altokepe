#include <QApplication>
#include "ui/VentanaRanking.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    VentanaRanking ventana;
    ventana.show();
    return app.exec();
}

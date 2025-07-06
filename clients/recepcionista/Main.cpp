#include <QApplication>
#include "MainWindow.h"
#include "RankingWindow.h"  // 👈 Asegúrate de tener este include

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Ventana principal (recepcionista o gestión)
    MainWindow window;
    window.showMaximized();  // o window.show();

    // Ventana del ranking y menú (ventana aparte)
    RankingWindow rankingWin;
    rankingWin.show();       // Siempre visible

    return app.exec();
}

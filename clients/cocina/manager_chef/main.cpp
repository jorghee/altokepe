#include "ui/VentanaManager.h"
#include <QApplication>
#include <QFile>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);

  // Cargar la hoja de estilos
  QFile styleFile(":/styles.qss"); // Asumiendo que se añade a los recursos de Qt (qrc)
  // O para desarrollo rápido:
  // QFile styleFile("ruta/a/tu/proyecto/clients/cocina/manager_chef/styles.qss");
  
  if (styleFile.open(QFile::ReadOnly)) {
    QString styleSheet = QLatin1String(styleFile.readAll());
    a.setStyleSheet(styleSheet);
    styleFile.close();
  } else {
    qWarning() << "No se pudo cargar la hoja de estilos.";
  }

  VentanaManager w;
  w.show();

  return a.exec();
}

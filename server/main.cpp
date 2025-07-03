#include <QCoreApplication>
#include "Servidor.h"
#include "LogicaNegocio.h"
#include <QTimer>
#include <QDebug>

int main(int argc, char* argv[]) {
  QCoreApplication a(argc, argv);
  a.setApplicationName("Servidor Altokepe");

  LogicaNegocio::instance()->cargarMenuDesdeArchivo("menu.json");

  Servidor servidor;
  quint16 port = 5555;

  if (!servidor.listen(QHostAddress::Any, port)) {
    qCritical() << "No se pudo iniciar el servidor en el puerto" << port;
    return 1;
  }

  qInfo() << "Servidor escuchando en el puerto" << port;

  QTimer::singleShot(500, [](){
    LogicaNegocio::instance()->simularRecepcionDePedidos();
  });

  return a.exec();
}

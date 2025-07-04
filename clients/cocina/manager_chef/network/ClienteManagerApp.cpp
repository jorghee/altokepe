#include "ClienteManagerApp.h"
#include "../ui/VentanaManager.h"
#include "../network/ClienteTCP.h"
#include "common/network/Protocolo.h"
#include "common/network/SerializadorJSON.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

ClienteManagerApp::ClienteManagerApp(QObject* parent) : QObject(parent) {
  m_ventana = new VentanaManager();
  m_clienteTCP = new ClienteTCP(this);

  // Conexion de Red
  connect(m_clienteTCP, &ClienteTCP::nuevoMensajeRecibido, this, &ClienteManagerApp::onMensajeRecibido);

  // Conexiones de Acción (UI -> Lógica)
  connect(m_ventana, &VentanaManager::prepararPedidoSolicitado, this, &ClienteManagerApp::onPrepararPedido);
  connect(m_ventana, &VentanaManager::cancelarPedidoSolicitado, this, &ClienteManagerApp::onCancelarPedido);
  connect(m_ventana, &VentanaManager::enviarPedidoSolicitado, this, &ClienteManagerApp::onEnviarPedido);
  connect(m_ventana, &VentanaManager::rechazarPedidoSolicitado, this, &ClienteManagerApp::onRechazarPedido);
}

void ClienteManagerApp::iniciar() {
  m_ventana->show();
  m_clienteTCP->conectar("127.0.0.1", 5555); // Conectar a localhost
}

void ClienteManagerApp::onMensajeRecibido(const QJsonObject& mensaje) {
  QString evento = mensaje[Protocolo::EVENTO].toString();

  if (evento == Protocolo::ACTUALIZACION_ESTADO_GENERAL) {
    QJsonObject data = mensaje[Protocolo::DATA].toObject();

    if (data.contains("menu")) {
      m_menu.clear();
      QJsonArray menuArray = data["menu"].toArray();
      for (const QJsonValue& val : menuArray) {
        PlatoDefinicion plato = SerializadorJSON::jsonToPlatoDefinicion(val.toObject());
        m_menu[plato.id] = plato;
      }
      qDebug() << "Menú recibido y almacenado con" << m_menu.size() << "platos.";
    }

    auto deserializarPedidos = [](const QJsonObject& data, const QString& clave) {
      std::vector<PedidoMesa> pedidos;
      QJsonArray pedidosJson = data[clave].toArray();
      for (const QJsonValue& val : pedidosJson) {
        pedidos.push_back(SerializadorJSON::jsonToPedidoMesa(val.toObject()));
      }
      return pedidos;
    };

    std::vector<PedidoMesa> pendientes = deserializarPedidos(data, "pedidos_pendientes");
    std::vector<PedidoMesa> enProgreso = deserializarPedidos(data, "pedidos_en_progreso");
    std::vector<PedidoMesa> terminados = deserializarPedidos(data, "pedidos_terminados");
    
    m_ventana->actualizarVistas(pendientes, enProgreso, terminados, m_menu);
  }
}

void ClienteManagerApp::onPrepararPedido(long long idPedido) {
  qDebug() << "Solicitando preparar pedido con ID:" << idPedido;
  QJsonObject comando;
  comando[Protocolo::COMANDO] = Protocolo::PREPARAR_PEDIDO;
  comando["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
  m_clienteTCP->enviarMensaje(comando);
}

void ClienteManagerApp::onCancelarPedido(long long idPedido) {
  qDebug() << "Solicitando cancelar pedido con ID:" << idPedido;
  QJsonObject comando;
  comando[Protocolo::COMANDO] = Protocolo::CANCELAR_PEDIDO;
  comando["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
  m_clienteTCP->enviarMensaje(comando);
}

void ClienteManagerApp::onEnviarPedido(long long idPedido) {
  qDebug() << "Solicitando ENVIAR platos del pedido:" << idPedido;
  QJsonObject comando;
  comando[Protocolo::COMANDO] = Protocolo::CONFIRMAR_ENTREGA;
  comando["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
  m_clienteTCP->enviarMensaje(comando);
}

void ClienteManagerApp::onRechazarPedido(long long idPedido) {
  qDebug() << "Solicitando RECHAZAR/DEVOLVER platos del pedido:" << idPedido;
  QJsonObject comando;
  comando[Protocolo::COMANDO] = Protocolo::DEVOLVER_PLATO;
  comando["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
  // En una implementación más detallada, se podría especificar qué plato devolver.
  // Por ahora, se asume que se devuelven todos los que no estén entregados.
  m_clienteTCP->enviarMensaje(comando);
}

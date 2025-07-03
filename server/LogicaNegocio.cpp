#include "LogicaNegocio.h"
#include "common/network/Protocolo.h"
#include "common/network/SerializadorJSON.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <chrono>
#include <unordered_set>

LogicaNegocio* LogicaNegocio::s_instance = nullptr;

LogicaNegocio::LogicaNegocio(QObject* parent) 
  : QObject(parent), m_siguienteIdPedido(1), m_siguienteIdInstanciaPlato(1) {
}

LogicaNegocio* LogicaNegocio::instance() {
  if (s_instance == nullptr) {
    s_instance = new LogicaNegocio();
  }
  return s_instance;
}

void LogicaNegocio::cargarMenuDesdeArchivo(const QString& rutaArchivo) {
  std::lock_guard<std::mutex> lock(m_mutex);
  QFile archivo(rutaArchivo);
  if (!archivo.open(QIODevice::ReadOnly)) {
    qCritical() << "No se pudo abrir el archivo de menú:" << rutaArchivo;
    return;
  }

  QByteArray data = archivo.readAll();
  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (!doc.isArray()) {
    qCritical() << "El archivo de menú no es un array JSON válido.";
    return;
  }

  m_menu.clear();
  QJsonArray menuArray = doc.array();
  for (const QJsonValue& val : menuArray) {
    PlatoDefinicion plato = SerializadorJSON::jsonToPlatoDefinicion(val.toObject());
    m_menu[plato.id] = plato;
  }
  qInfo() << "Menú cargado desde" << rutaArchivo << "con" << m_menu.size() << "platos.";
}

void LogicaNegocio::procesarMensaje(const QJsonObject& mensaje) {
  QString comando = mensaje[Protocolo::COMANDO].toString();

  if (comando == Protocolo::NUEVO_PEDIDO) {
    procesarNuevoPedido(mensaje);
  } else if (comando == Protocolo::PREPARAR_PEDIDO) {
    procesarPrepararPedido(mensaje);
  } else if (comando == Protocolo::CANCELAR_PEDIDO) {
    procesarCancelarPedido(mensaje);
  } else if (comando == Protocolo::CONFIRMAR_ENTREGA) {
    procesarConfirmarEntrega(mensaje);
  } else if (comando == Protocolo::DEVOLVER_PLATO) {
    procesarDevolverPlato(mensaje);
  } else {
    qWarning() << "Comando desconocido recibido:" << comando;
  }
}

void LogicaNegocio::procesarNuevoPedido(const QJsonObject& data) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    PedidoMesa nuevoPedido;
    nuevoPedido.id_pedido = m_siguienteIdPedido++;
    nuevoPedido.numero_mesa = data["mesa"].toInt();
    nuevoPedido.id_recepcionista = data["id_recepcionista"].toInt();
    nuevoPedido.timestamp_creacion = std::chrono::system_clock::now();
    nuevoPedido.estado_general = EstadoPedido::ACTIVO;
    QJsonArray platosArray = data["platos"].toArray();
    for (const QJsonValue& val : platosArray) {
      QJsonObject platoObj = val.toObject();
      int idPlatoDefinicion = platoObj["id"].toInt();
      int cantidad = platoObj["cantidad"].toInt();
      for (int i = 0; i < cantidad; ++i) {
        PlatoInstancia instancia;
        instancia.id_instancia = m_siguienteIdInstanciaPlato++;
        instancia.id_plato_definicion = idPlatoDefinicion;
        instancia.estado = EstadoPlato::EN_ESPERA;
        instancia.timestamp_creacion = std::chrono::system_clock::now();
        instancia.timestamp_ultimo_cambio = instancia.timestamp_creacion;
        nuevoPedido.platos.push_back(instancia);
        m_conteoPlatosRanking[idPlatoDefinicion]++;
      }
    }
    m_pedidosActivos[nuevoPedido.id_pedido] = nuevoPedido;
    m_colaManagerChef.push(nuevoPedido.id_pedido);
    qDebug() << "Nuevo pedido" << nuevoPedido.id_pedido << "para mesa" << nuevoPedido.numero_mesa << "creado.";
  }
  emit broadcast(getEstadoCompleto());
}

void LogicaNegocio::procesarPrepararPedido(const QJsonObject& data) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    long long idPedido = data["id_pedido"].toVariant().toLongLong();
    if (m_colaManagerChef.empty() || m_colaManagerChef.front() != idPedido) {
      qWarning() << "Intento de preparar un pedido que no está al frente de la cola.";
      return;
    }
    if (m_pedidosActivos.count(idPedido) == 0) return;
    m_colaManagerChef.pop();
    PedidoMesa& pedido = m_pedidosActivos.at(idPedido);
    for (auto& platoInstancia : pedido.platos) {
      platoInstancia.estado = EstadoPlato::EN_PROGRESO; // Simulación: todos a "en progreso"
      const auto& platoDef = m_menu.at(platoInstancia.id_plato_definicion);
      double W1 = 1.0, W2 = 0.5;
      auto antiguedad = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - pedido.timestamp_creacion).count();
      double score = (W1 * platoDef.tiempo_preparacion_estimado) - (W2 * antiguedad);
      InfoPlatoPrioridad info {pedido.id_pedido, platoInstancia.id_instancia, score};
      m_colasPorEstacion[platoDef.estacion].push(info);
    }
    qDebug() << "Pedido" << idPedido << "distribuido a las estaciones.";
  }
  emit broadcast(getEstadoCompleto());
}

void LogicaNegocio::procesarCancelarPedido(const QJsonObject& data) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    long long idPedido = data["id_pedido"].toVariant().toLongLong();

    if (m_colaManagerChef.empty() || m_colaManagerChef.front() != idPedido) return;
    if (m_pedidosActivos.count(idPedido) == 0) return;
    
    m_colaManagerChef.pop();
    PedidoMesa& pedido = m_pedidosActivos.at(idPedido);
    pedido.estado_general = EstadoPedido::CANCELADO;
    for(auto& plato : pedido.platos) {
      plato.estado = EstadoPlato::CANCELADO;
    }
    qDebug() << "Pedido" << idPedido << "cancelado.";
  }
  emit broadcast(getEstadoCompleto());
}

void LogicaNegocio::procesarConfirmarEntrega(const QJsonObject& data) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    long long idPedido = data["id_pedido"].toVariant().toLongLong();
    if (m_pedidosActivos.count(idPedido) == 0) return;
    PedidoMesa& pedido = m_pedidosActivos.at(idPedido);
    for(auto& plato : pedido.platos) {
      if (plato.estado == EstadoPlato::FINALIZADO) {
        plato.estado = EstadoPlato::ENTREGADO;
      }
    }
    qDebug() << "Pedido" << idPedido << "marcado como entregado.";
  }
  emit broadcast(getEstadoCompleto());
}

void LogicaNegocio::procesarDevolverPlato(const QJsonObject& data) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    long long idPedido = data["id_pedido"].toVariant().toLongLong();
    if (m_pedidosActivos.count(idPedido) == 0) return;
    PedidoMesa& pedido = m_pedidosActivos.at(idPedido);
    // Lógica simplificada: se devuelve el primer plato finalizado para retrabajo.
    for(auto& plato : pedido.platos) {
      if (plato.estado == EstadoPlato::FINALIZADO) {
        plato.estado = EstadoPlato::EN_PROGRESO; // Lo devolvemos a preparación
        const auto& platoDef = m_menu.at(plato.id_plato_definicion);
        InfoPlatoPrioridad info {pedido.id_pedido, plato.id_instancia, -9999.0}; // Máxima prioridad
        m_colasPorEstacion[platoDef.estacion].push(info);
        qDebug() << "Plato" << plato.id_instancia << "del pedido" << idPedido << "devuelto a cocina.";
        break; // Solo devolvemos uno para este ejemplo
      }
    }
  }
  emit broadcast(getEstadoCompleto());
}

void LogicaNegocio::clasificarPedidos(std::vector<PedidoMesa>& pendientes, std::vector<PedidoMesa>& enProgreso,
                                    std::vector<PedidoMesa>& terminados) {
  std::unordered_set<long long> idsPendientes;
  std::queue<long long> tempQueue = m_colaManagerChef;
  while (!tempQueue.empty()) {
    idsPendientes.insert(tempQueue.front());
    tempQueue.pop();
  }

  for (const auto& par : m_pedidosActivos) {
    const PedidoMesa& pedido = par.second;
    if (pedido.estado_general == EstadoPedido::CANCELADO) continue;
    if (idsPendientes.count(pedido.id_pedido)) {
      pendientes.push_back(pedido);
      continue;
    }

    bool todoTerminado = true;
    bool algunEnProgreso = false;
    for (const auto& plato : pedido.platos) {
      if (plato.estado != EstadoPlato::FINALIZADO && plato.estado != EstadoPlato::ENTREGADO) {
        todoTerminado = false;
      }
      if (plato.estado == EstadoPlato::EN_ESPERA || plato.estado == EstadoPlato::EN_PROGRESO) {
        algunEnProgreso = true;
      }
    }
    if (todoTerminado) {
      terminados.push_back(pedido);
    } else if (algunEnProgreso) {
      enProgreso.push_back(pedido);
    }
  }
}

QJsonObject LogicaNegocio::getEstadoCompleto() {
  QJsonObject estado;
  std::vector<PedidoMesa> pendientes, enProgreso, terminados;
  clasificarPedidos(pendientes, enProgreso, terminados);

  QJsonObject data;
  QJsonArray arrPendientes, arrEnProgreso, arrTerminados;
  for(const auto& p : pendientes) arrPendientes.append(SerializadorJSON::pedidoMesaToJson(p));
  for(const auto& p : enProgreso) arrEnProgreso.append(SerializadorJSON::pedidoMesaToJson(p));
  for(const auto& p : terminados) arrTerminados.append(SerializadorJSON::pedidoMesaToJson(p));
  data["pedidos_pendientes"] = arrPendientes;
  data["pedidos_en_progreso"] = arrEnProgreso;
  data["pedidos_terminados"] = arrTerminados;

  estado[Protocolo::EVENTO] = Protocolo::ACTUALIZACION_ESTADO_GENERAL;
  estado[Protocolo::DATA] = data;
 
  return estado;
}

QJsonObject LogicaNegocio::getMenuCompleto() {
  QJsonObject menuData;
  QJsonArray menuArray;
  for(const auto& par : m_menu) {
    menuArray.append(SerializadorJSON::platoDefinicionToJson(par.second));
  }
  menuData["menu"] = menuArray;
  return menuData;
}

void LogicaNegocio::simularRecepcionDePedidos() {
  qDebug() << "--- INICIANDO SIMULACIÓN DE RECEPCIÓN DE PEDIDOS ---";

  {
    QJsonObject p1;
    p1[Protocolo::COMANDO] = Protocolo::NUEVO_PEDIDO;
    p1["mesa"] = 4;
    p1["id_recepcionista"] = 101;
    QJsonArray platos1;
    QJsonObject plato1_1; plato1_1["id"] = 101; plato1_1["cantidad"] = 1;
    QJsonObject plato1_2; plato1_2["id"] = 201; plato1_2["cantidad"] = 2;
    platos1.append(plato1_1);
    platos1.append(plato1_2);
    p1["platos"] = platos1;
    procesarMensaje(p1);
  }

  {
    QJsonObject p2;
    p2[Protocolo::COMANDO] = Protocolo::NUEVO_PEDIDO;
    p2["mesa"] = 1;
    p2["id_recepcionista"] = 102;
    QJsonArray platos2;
    QJsonObject plato2_1; plato2_1["id"] = 102; plato2_1["cantidad"] = 1;
    platos2.append(plato2_1);
    p2["platos"] = platos2;
    procesarMensaje(p2);
  }

  qDebug() << "--- SIMULACIÓN FINALIZADA ---";
}



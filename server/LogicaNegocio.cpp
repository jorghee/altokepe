#include "LogicaNegocio.h"
#include "ManejadorCliente.h"
#include "common/network/Protocolo.h"
#include "common/models/Estados.h"
#include "common/network/SerializadorJSON.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <chrono>
#include <algorithm>


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

void LogicaNegocio::registrarManejador(ManejadorCliente* manejador) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_manejadoresActivos.push_back(manejador);
}

void LogicaNegocio::eliminarManejador(ManejadorCliente* manejador) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_manejadoresActivos.erase(std::remove(m_manejadoresActivos.begin(),
        m_manejadoresActivos.end(), manejador), m_manejadoresActivos.end());
}

void LogicaNegocio::enviarEstadoInicial(ManejadorCliente* cliente) {
  std::lock_guard<std::mutex> lock(m_mutex);
  QJsonObject estado;
  TipoActor tipo = cliente->getTipoActor();

  if (tipo == TipoActor::MANAGER_CHEF || tipo == TipoActor::RANKING) {
    estado = getEstadoParaManagerYRanking(true);
  } else if (tipo == TipoActor::ESTACION_COCINA) {
    estado = getEstadoParaEstacion(cliente->getNombreEstacion().toStdString());
  } else if (tipo == TipoActor::RECEPCIONISTA) {
    QJsonObject mensaje;
    QJsonArray menuArray;

    for (const auto& par : m_menu) {
      const PlatoDefinicion& plato = par.second;
      QJsonObject platoJson;
      platoJson["id"] = plato.id;
      platoJson["nombre"] = QString::fromStdString(plato.nombre);
      platoJson["precio"] = plato.costo;
      platoJson["tiempo_preparacion"] = plato.tiempo_preparacion_estimado;
      platoJson["categoria"] = QString::fromStdString(plato.estacion);
      menuArray.append(platoJson);
    }

    mensaje[Protocolo::EVENTO] = "ACTUALIZACION_MENU";
    mensaje[Protocolo::DATA] = QJsonObject{ {"menu", menuArray} };

    emit enviarRespuesta(cliente, mensaje);
    return; // Ya enviamos el estado especial para el recepcionista, salimos
  }

  if (!estado.isEmpty()) {
    emit enviarRespuesta(cliente, estado);
  }
}

void LogicaNegocio::procesarMensaje(const QJsonObject& mensaje, ManejadorCliente* remitente) {
  QString comando = mensaje[Protocolo::COMANDO].toString();

  if (comando == Protocolo::NUEVO_PEDIDO) {
    procesarNuevoPedido(mensaje, remitente);
  } else if (comando == Protocolo::PREPARAR_PEDIDO) {
    procesarPrepararPedido(mensaje, remitente);
  } else if (comando == Protocolo::CANCELAR_PEDIDO) {
    procesarCancelarPedido(mensaje, remitente);
  } else if (comando == Protocolo::MARCAR_PLATO_TERMINADO) {
    procesarMarcarPlatoTerminado(mensaje, remitente);
  } else if (comando == Protocolo::CONFIRMAR_ENTREGA) {
    procesarConfirmarEntrega(mensaje, remitente);
  } else if (comando == Protocolo::DEVOLVER_PLATO) {
    procesarDevolverPlato(mensaje, remitente);
  } else if (comando == "SOLICITAR_ESTADO") {
    enviarEstadoInicial(remitente);  // <<=== NUEVO
  }else{
    qWarning() << "Comando desconocido recibido:" << comando;
    return;
  }
}

void LogicaNegocio::procesarNuevoPedido(const QJsonObject& data, ManejadorCliente* remitente) {
  std::lock_guard<std::mutex> lock(m_mutex);

  PedidoMesa nuevoPedido;
  nuevoPedido.id_pedido = m_siguienteIdPedido++;
  nuevoPedido.numero_mesa = data["mesa"].toInt();
  nuevoPedido.id_recepcionista = data["id_recepcionista"].toInt();
  nuevoPedido.timestamp_creacion = std::chrono::system_clock::now();
  nuevoPedido.estado_general = EstadoPedido::PENDIENTE;

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

  QJsonObject notificacion;
  notificacion[Protocolo::EVENTO] = "PEDIDO_NUEVO";
  notificacion[Protocolo::DATA] = SerializadorJSON::pedidoMesaToJson(nuevoPedido);
  notificarManagersYRanking_unlocked(notificacion);
}

void LogicaNegocio::procesarPrepararPedido(const QJsonObject& data, ManejadorCliente* remitente) {
  std::lock_guard<std::mutex> lock(m_mutex);
  long long idPedido = data["id_pedido"].toVariant().toLongLong();

  if (m_colaManagerChef.empty() || m_colaManagerChef.front() != idPedido) return;
  if (m_pedidosActivos.count(idPedido) == 0) return;

  m_colaManagerChef.pop();
  PedidoMesa& pedido = m_pedidosActivos.at(idPedido);
  pedido.estado_general = EstadoPedido::EN_PROGRESO;

  QJsonObject notificacionManager;
  notificacionManager[Protocolo::EVENTO] = "PEDIDO_A_PROGRESO";
  QJsonObject dataManager;
  dataManager["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
  notificacionManager[Protocolo::DATA] = dataManager;
  notificarManagersYRanking_unlocked(notificacionManager);

  for (auto& platoInstancia : pedido.platos) {
    platoInstancia.estado = EstadoPlato::EN_PROGRESO;
    const auto& platoDef = m_menu.at(platoInstancia.id_plato_definicion);

    double W1 = 1.0, W2 = 0.5;
    auto antiguedad = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - 
        pedido.timestamp_creacion).count();
    double score = (W1 * platoDef.tiempo_preparacion_estimado) - (W2 * antiguedad);

    InfoPlatoPrioridad info {pedido.id_pedido, platoInstancia.id_instancia, score};
    m_colasPorEstacion[platoDef.estacion].push(info);

    QJsonObject notificacionEstacion;
    notificacionEstacion[Protocolo::EVENTO] = "NUEVO_PLATO_EN_COLA";
    QJsonObject dataPlato;
    dataPlato["nombre"] = QString::fromStdString(platoDef.nombre);
    dataPlato["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(info.id_pedido));
    dataPlato["id_instancia"] = QJsonValue::fromVariant(QVariant::fromValue(info.id_instancia_plato));
    dataPlato["score"] = info.score_prioridad;
    notificacionEstacion[Protocolo::DATA] = dataPlato;
    notificarEstacion_unlocked(platoDef.estacion, notificacionEstacion);
  }
  qDebug() << "Pedido" << idPedido << "distribuido a las estaciones.";
}

void LogicaNegocio::procesarCancelarPedido(const QJsonObject& data, ManejadorCliente* remitente) {
  std::lock_guard<std::mutex> lock(m_mutex);
  long long idPedido = data["id_pedido"].toVariant().toLongLong();

  if (m_colaManagerChef.empty() || m_colaManagerChef.front() != idPedido) return;
  if (m_pedidosActivos.count(idPedido) == 0) return;
  
  m_colaManagerChef.pop();
  PedidoMesa& pedido = m_pedidosActivos.at(idPedido);
  pedido.estado_general = EstadoPedido::CANCELADO;
  for(auto& plato : pedido.platos) plato.estado = EstadoPlato::CANCELADO;
  qDebug() << "Pedido" << idPedido << "cancelado.";

  QJsonObject notificacion;
  notificacion[Protocolo::EVENTO] = "PEDIDO_CANCELADO";
  QJsonObject dataNotif;
  dataNotif["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
  notificacion[Protocolo::DATA] = dataNotif;
  notificarManagersYRanking_unlocked(notificacion);

  m_pedidosActivos.erase(idPedido);
}

void LogicaNegocio::procesarMarcarPlatoTerminado(const QJsonObject& data, ManejadorCliente* remitente) {
  std::lock_guard<std::mutex> lock(m_mutex);

  long long idPedido = data["id_pedido"].toVariant().toLongLong();
  long long idInstancia = data["id_instancia"].toVariant().toLongLong();

  if (!remitente || remitente->getTipoActor() != TipoActor::ESTACION_COCINA) {
    qWarning() << "Intento de marcar plato terminado por un cliente que no es EstacionCocina. Ignorando.";
    return;
  }
  std::string nombreEstacion = remitente->getNombreEstacion().toStdString(); 

  if (m_pedidosActivos.count(idPedido) == 0) return;

  auto& pedido = m_pedidosActivos.at(idPedido);
  bool todosTerminados = true;
  bool platoEncontrado = false;

  for (auto& plato : pedido.platos) {
    if (plato.id_instancia == idInstancia) {
      const auto& platoDef = m_menu.at(plato.id_plato_definicion);
      if (platoDef.estacion != nombreEstacion) {
        qWarning() << "Estación" << QString::fromStdString(nombreEstacion) 
                   << "intentó finalizar un plato que no le pertenece (Pertenece a"
                   << QString::fromStdString(platoDef.estacion) << ").";
        return; // Ignorar la solicitud por seguridad.
      }
      plato.estado = EstadoPlato::FINALIZADO;
      plato.timestamp_ultimo_cambio = std::chrono::system_clock::now();
      platoEncontrado = true;
    }
    if (plato.estado != EstadoPlato::FINALIZADO && plato.estado != EstadoPlato::ENTREGADO &&
        plato.estado != EstadoPlato::CANCELADO) {
      todosTerminados = false;
    }
  }

  if(!platoEncontrado) return;

  QJsonObject notifActualizacionPlato;
  notifActualizacionPlato[Protocolo::EVENTO] = "PLATO_ESTADO_CAMBIADO";
  QJsonObject dataPlato;
  dataPlato["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
  dataPlato["id_instancia"] = QJsonValue::fromVariant(QVariant::fromValue(idInstancia));
  dataPlato["nuevo_estado"] = SerializadorJSON::estadoPlatoToString(EstadoPlato::FINALIZADO);
  notifActualizacionPlato[Protocolo::DATA] = dataPlato;
  notificarManagersYRanking_unlocked(notifActualizacionPlato);
  notificarEstacion_unlocked(nombreEstacion, notifActualizacionPlato);

  if (todosTerminados) {
    pedido.estado_general = EstadoPedido::COMPLETADO;

    QJsonObject notifPedidoCompletado;
    notifPedidoCompletado[Protocolo::EVENTO] = "PEDIDO_COMPLETADO";
    QJsonObject dataPedido;
    dataPedido["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
    notifPedidoCompletado[Protocolo::DATA] = dataPedido;
    notificarManagersYRanking_unlocked(notifPedidoCompletado);
  }
}

void LogicaNegocio::procesarConfirmarEntrega(const QJsonObject& data, ManejadorCliente* remitente) {
  std::lock_guard<std::mutex> lock(m_mutex);
  long long idPedido = data["id_pedido"].toVariant().toLongLong();

  if (m_pedidosActivos.count(idPedido) == 0) return;

  PedidoMesa& pedido = m_pedidosActivos.at(idPedido);
  int idRecepcionista = pedido.id_recepcionista;
  bool platoListo = false;
  for(auto& plato : pedido.platos) {
    if (plato.estado == EstadoPlato::FINALIZADO) {
      plato.estado = EstadoPlato::ENTREGADO;
      platoListo = true;
    }
  }
  if (platoListo) {
    notificarRecepcionista_unlocked(idRecepcionista, idPedido);
  }

  QJsonObject notificacion;
  notificacion[Protocolo::EVENTO] = "PEDIDO_ENTREGADO";
  QJsonObject dataNotif;
  dataNotif["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
  notificacion[Protocolo::DATA] = dataNotif;
  notificarManagersYRanking_unlocked(notificacion);
}

void LogicaNegocio::procesarDevolverPlato(const QJsonObject& data, ManejadorCliente* remitente) {
  std::lock_guard<std::mutex> lock(m_mutex);

  long long idPedido = data["id_pedido"].toVariant().toLongLong();
  // Asumimos que se devuelve el primer plato finalizado que se encuentre
  long long idInstanciaDevuelta = -1; 

  if (m_pedidosActivos.count(idPedido) == 0) return;

  PedidoMesa& pedido = m_pedidosActivos.at(idPedido);
  pedido.estado_general = EstadoPedido::EN_PROGRESO;

  for(auto& plato : pedido.platos) {
    if (plato.estado == EstadoPlato::FINALIZADO) {
      plato.estado = EstadoPlato::EN_PROGRESO;
      idInstanciaDevuelta = plato.id_instancia;
      const auto& platoDef = m_menu.at(plato.id_plato_definicion);

      InfoPlatoPrioridad info {pedido.id_pedido, plato.id_instancia, -9999.0};
      m_colasPorEstacion[platoDef.estacion].push(info);
      qDebug() << "Plato" << plato.id_instancia << "del pedido" << idPedido << "devuelto a cocina.";

      QJsonObject notificacionEstacion;
      notificacionEstacion[Protocolo::EVENTO] = "NUEVO_PLATO_EN_COLA";
      QJsonObject dataPlatoEstacion;
      dataPlatoEstacion["nombre"] = QString::fromStdString(platoDef.nombre);
      dataPlatoEstacion["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(info.id_pedido));
      dataPlatoEstacion["id_instancia"] = QJsonValue::fromVariant(QVariant::fromValue(info.id_instancia_plato));
      dataPlatoEstacion["score"] = info.score_prioridad;
      notificacionEstacion[Protocolo::DATA] = dataPlatoEstacion;
      notificarEstacion_unlocked(platoDef.estacion, notificacionEstacion);

      break; // Solo devolvemos un plato a la vez
    }
  }

  if (idInstanciaDevuelta == -1) return; 

  QJsonObject notifMover;
  notifMover[Protocolo::EVENTO] = "PEDIDO_A_PROGRESO";
  QJsonObject dataMover;
  dataMover["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
  notifMover[Protocolo::DATA] = dataMover;
  notificarManagersYRanking_unlocked(notifMover);

  QJsonObject notifActualizarPlato;
  notifActualizarPlato[Protocolo::EVENTO] = "PLATO_ESTADO_CAMBIADO";
  QJsonObject dataPlatoManager;
  dataPlatoManager["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
  dataPlatoManager["id_instancia"] = QJsonValue::fromVariant(QVariant::fromValue(idInstanciaDevuelta));
  dataPlatoManager["nuevo_estado"] = SerializadorJSON::estadoPlatoToString(EstadoPlato::EN_PROGRESO);
  notifActualizarPlato[Protocolo::DATA] = dataPlatoManager;
  notificarManagersYRanking_unlocked(notifActualizarPlato);
}

void LogicaNegocio::clasificarPedidos(std::vector<PedidoMesa>& pendientes,
    std::vector<PedidoMesa>& enProgreso, std::vector<PedidoMesa>& terminados) {
  for (const auto& par : m_pedidosActivos) {
    const PedidoMesa& pedido = par.second;
    switch(pedido.estado_general) {
      case EstadoPedido::PENDIENTE:
        pendientes.push_back(pedido);
        break;
      case EstadoPedido::EN_PROGRESO:
        enProgreso.push_back(pedido);
        break;
      case EstadoPedido::COMPLETADO:
        terminados.push_back(pedido);
        break;
      case EstadoPedido::CANCELADO:
        break;
    }
  }
  std::sort(pendientes.begin(), pendientes.end(), [](const PedidoMesa& a, const PedidoMesa& b) {
    return a.timestamp_creacion < b.timestamp_creacion;
  });
}

QJsonObject LogicaNegocio::getEstadoParaManagerYRanking(bool incluirMenu) {
  QJsonObject mensaje;
  QJsonObject data;
  std::vector<PedidoMesa> pendientes, enProgreso, terminados;
  
  clasificarPedidos(pendientes, enProgreso, terminados);

  auto toJsonArray = [](const std::vector<PedidoMesa>& pedidos) {
    QJsonArray arr;
    for(const auto& p : pedidos) arr.append(SerializadorJSON::pedidoMesaToJson(p));
    return arr;
  };
  
  data["pedidos_pendientes"] = toJsonArray(pendientes);
  data["pedidos_en_progreso"] = toJsonArray(enProgreso);
  data["pedidos_terminados"] = toJsonArray(terminados);
  
  QJsonArray rankingArray;
  for(const auto& par : m_conteoPlatosRanking){
    auto it = m_menu.find(par.first);
    if (it != m_menu.end()) {
        QJsonObject item;
        item["nombre"] = QString::fromStdString(it->second.nombre);
        item["cantidad"] = par.second;
        item["precio"] = it->second.costo;
        rankingArray.append(item);
    } else {
        qWarning() << "ID de plato no encontrado en el menú:" << par.first;
    }
}

  
  data["ranking"] = rankingArray;

  if (incluirMenu) {
    QJsonArray menuArray;
    for(const auto& par : m_menu) {
      menuArray.append(SerializadorJSON::platoDefinicionToJson(par.second));
    }
    data["menu"] = menuArray;
  }

  mensaje[Protocolo::EVENTO] = Protocolo::ACTUALIZACION_ESTADO_GENERAL;
  mensaje[Protocolo::DATA] = data;
 
  return mensaje;
}

QJsonObject LogicaNegocio::getEstadoParaEstacion(const std::string& nombreEstacion) {
  QJsonObject mensaje;
  QJsonObject data;
  QJsonArray platosPendientes;

  if (m_colasPorEstacion.count(nombreEstacion)) {
    // Creamos una copia de la cola de prioridad para no modificar la original
    ColaPrioridadPlatos tempQueue = m_colasPorEstacion.at(nombreEstacion);
    
    while (!tempQueue.empty()) {
      InfoPlatoPrioridad infoPlato = tempQueue.top();
      tempQueue.pop();

      // Buscamos la definición del plato para obtener su nombre
      if (m_pedidosActivos.count(infoPlato.id_pedido)) {
        const auto& pedido = m_pedidosActivos.at(infoPlato.id_pedido);
        for (const auto& instancia : pedido.platos) {
          if (instancia.id_instancia == infoPlato.id_instancia_plato) {
            const auto& definicion = m_menu.at(instancia.id_plato_definicion);
            QJsonObject platoJson;
            platoJson["nombre"] = QString::fromStdString(definicion.nombre);
            platoJson["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(infoPlato.id_pedido));
            platoJson["id_instancia"] = QJsonValue::fromVariant(QVariant::fromValue(infoPlato.id_instancia_plato));
            platoJson["score"] = infoPlato.score_prioridad;
            platosPendientes.append(platoJson);
            break; 
          }
        }
      }
    }
  }

  data["platos_pendientes"] = platosPendientes;
  mensaje[Protocolo::EVENTO] = Protocolo::ACTUALIZACION_ESTADO_ESTACION;
  mensaje[Protocolo::DATA] = data;

  return mensaje;
}

void LogicaNegocio::notificarManagersYRanking(const QJsonObject& mensaje) {
  std::lock_guard<std::mutex> lock(m_mutex);
  notificarManagersYRanking_unlocked(mensaje);
}

void LogicaNegocio::notificarEstacion(const std::string& nombreEstacion, const QJsonObject& mensaje) {
  std::lock_guard<std::mutex> lock(m_mutex);
  notificarEstacion_unlocked(nombreEstacion, mensaje);
}

void LogicaNegocio::notificarRecepcionista(int idRecepcionista, long long idPedido) {
  std::lock_guard<std::mutex> lock(m_mutex);
  notificarRecepcionista_unlocked(idRecepcionista, idPedido);
}

void LogicaNegocio::notificarManagersYRanking_unlocked(const QJsonObject& mensaje) {
  for (ManejadorCliente* manejador : m_manejadoresActivos) {
    TipoActor tipo = manejador->getTipoActor();
    if (tipo == TipoActor::MANAGER_CHEF || tipo == TipoActor::RANKING) {
      emit enviarRespuesta(manejador, mensaje);
    }
  }
}

void LogicaNegocio::notificarEstacion_unlocked(const std::string& nombreEstacion, const QJsonObject& mensaje) {
  for (ManejadorCliente* manejador : m_manejadoresActivos) {
    if (manejador->getTipoActor() == TipoActor::ESTACION_COCINA && 
        manejador->getNombreEstacion().toStdString() == nombreEstacion) {
      emit enviarRespuesta(manejador, mensaje);
      qDebug() << "Notificando a la estación:" << QString::fromStdString(nombreEstacion);
      break; 
    }
  }
}

void LogicaNegocio::notificarRecepcionista_unlocked(int idRecepcionista, long long idPedido) {
  QJsonObject notificacion;
  notificacion[Protocolo::EVENTO] = "PLATO_LISTO";
  QJsonObject data;
  data["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
  if (m_pedidosActivos.count(idPedido)) {
    data["mesa"] = m_pedidosActivos.at(idPedido).numero_mesa;
  }
  notificacion[Protocolo::DATA] = data;

  for (ManejadorCliente* manejador : m_manejadoresActivos) {
    if (manejador->getTipoActor() == TipoActor::RECEPCIONISTA && manejador->getIdActor() == idRecepcionista) {
      emit enviarRespuesta(manejador, notificacion);
      qDebug() << "Notificando a recepcionista" << idRecepcionista << "que el pedido" << idPedido << "tiene platos listos.";
      break; 
    }
  }
}

// Este método se eliminará, los datos se reciben del recepcionista
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
    procesarMensaje(p1, nullptr);
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
    procesarMensaje(p2, nullptr);
  }

  qDebug() << "--- SIMULACIÓN FINALIZADA ---";
}
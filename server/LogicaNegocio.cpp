#include "LogicaNegocio.h"
#include "ManejadorCliente.h"
#include "common/network/Protocolo.h"
#include "common/network/SerializadorJSON.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <chrono>

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
  QString rol = cliente->getRol();

  if (rol == "ManagerChef" || rol == "Ranking") {
    estado = getEstadoParaManagerYRanking();
  } else if (rol == "EstacionCocina") {
    // Necesitaríamos el nombre de la estación aquí
    // estado = getEstadoParaEstacion(cliente->getNombreEstacion());
  }

  if (!estado.isEmpty()) {
    emit enviarRespuesta(cliente, estado);
  }
}

void LogicaNegocio::procesarMensaje(const QJsonObject& mensaje, ManejadorCliente* remitente) {
  QString comando = mensaje[Protocolo::COMANDO].toString();
  std::lock_guard<std::mutex> lock(m_mutex);

  if (comando == Protocolo::NUEVO_PEDIDO) {
    procesarNuevoPedido(mensaje, remitente);
    notificarManagersYRanking();
  } else if (comando == Protocolo::PREPARAR_PEDIDO) {
    procesarPrepararPedido(mensaje, remitente);
    notificarManagersYRanking();
  } else if (comando == Protocolo::CANCELAR_PEDIDO) {
    procesarCancelarPedido(mensaje, remitente);
    notificarManagersYRanking();
  } else if (comando == Protocolo::MARCAR_PLATO_TERMINADO) {
    procesarMarcarPlatoTerminado(mensaje, remitente);
    notificarManagersYRanking();
  } else if (comando == Protocolo::CONFIRMAR_ENTREGA) {
    procesarConfirmarEntrega(mensaje, remitente);
    notificarManagersYRanking();
  } else if (comando == Protocolo::DEVOLVER_PLATO) {
    procesarDevolverPlato(mensaje, remitente);
    notificarManagersYRanking();
  } else {
    qWarning() << "Comando desconocido recibido:" << comando;
  }
}

void LogicaNegocio::procesarNuevoPedido(const QJsonObject& data, ManejadorCliente* remitente) {
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
}

void LogicaNegocio::procesarPrepararPedido(const QJsonObject& data, ManejadorCliente* remitente) {
  long long idPedido = data["id_pedido"].toVariant().toLongLong();

  if (m_colaManagerChef.empty() || m_colaManagerChef.front() != idPedido) return;
  if (m_pedidosActivos.count(idPedido) == 0) return;

  m_colaManagerChef.pop();
  PedidoMesa& pedido = m_pedidosActivos.at(idPedido);
  pedido.estado_general = EstadoPedido::EN_PROGRESO;

  std::unordered_set<std::string> estacionesAfectadas;

  for (auto& platoInstancia : pedido.platos) {
    platoInstancia.estado = EstadoPlato::EN_PROGRESO; // Simulación: todos a "en progreso"
    const auto& platoDef = m_menu.at(platoInstancia.id_plato_definicion);

    double W1 = 1.0, W2 = 0.5;
    auto antiguedad = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - 
        pedido.timestamp_creacion).count();
    double score = (W1 * platoDef.tiempo_preparacion_estimado) - (W2 * antiguedad);

    InfoPlatoPrioridad info {pedido.id_pedido, platoInstancia.id_instancia, score};
    m_colasPorEstacion[platoDef.estacion].push(info);

    estacionesAfectadas.insert(platoDef.estacion);
  }
  qDebug() << "Pedido" << idPedido << "distribuido a las estaciones.";

  for (const auto& nombreEstacion : estacionesAfectadas) {
    notificarEstacion(nombreEstacion);
  }
}

void LogicaNegocio::procesarCancelarPedido(const QJsonObject& data, ManejadorCliente* remitente) {
  long long idPedido = data["id_pedido"].toVariant().toLongLong();

  if (m_colaManagerChef.empty() || m_colaManagerChef.front() != idPedido) return;
  if (m_pedidosActivos.count(idPedido) == 0) return;
  
  m_colaManagerChef.pop();
  PedidoMesa& pedido = m_pedidosActivos.at(idPedido);
  pedido.estado_general = EstadoPedido::CANCELADO;
  for(auto& plato : pedido.platos) plato.estado = EstadoPlato::CANCELADO;
  qDebug() << "Pedido" << idPedido << "cancelado.";
}

void LogicaNegocio::procesarMarcarPlatoTerminado(const QJsonObject& data, ManejadorCliente* remitente) {
  long long idPedido = data["id_pedido"].toVariant().toLongLong();
  long long idInstancia = data["id_instancia"].toVariant().toLongLong();
  std::string nombreEstacion = remitente->getRol().toStdString(); 

  if (m_pedidosActivos.count(idPedido)) {
    auto& pedido = m_pedidosActivos.at(idPedido);
    bool todosTerminados = true;
    for (auto& plato : pedido.platos) {
      if (plato.id_instancia == idInstancia) {
        plato.estado = EstadoPlato::FINALIZADO;
        plato.timestamp_ultimo_cambio = std::chrono::system_clock::now();
      }
      if (plato.estado != EstadoPlato::FINALIZADO && plato.estado != EstadoPlato::ENTREGADO) {
        todosTerminados = false;
      }
    }
    if (todosTerminados) {
      pedido.estado_general = EstadoPedido::COMPLETADO;
    }
  }
  notificarEstacion(nombreEstacion);
}

void LogicaNegocio::procesarConfirmarEntrega(const QJsonObject& data, ManejadorCliente* remitente) {
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
    notificarRecepcionista(idRecepcionista, idPedido);
  }
}

void LogicaNegocio::procesarDevolverPlato(const QJsonObject& data, ManejadorCliente* remitente) {
  long long idPedido = data["id_pedido"].toVariant().toLongLong();
  if (m_pedidosActivos.count(idPedido) == 0) return;
  PedidoMesa& pedido = m_pedidosActivos.at(idPedido);
  pedido.estado_general = EstadoPedido::EN_PROGRESO;

  for(auto& plato : pedido.platos) {
    if (plato.estado == EstadoPlato::FINALIZADO) {
      plato.estado = EstadoPlato::EN_PROGRESO;
      const auto& platoDef = m_menu.at(plato.id_plato_definicion);
      InfoPlatoPrioridad info {pedido.id_pedido, plato.id_instancia, -9999.0}; // Máxima prioridad
      m_colasPorEstacion[platoDef.estacion].push(info);
      qDebug() << "Plato" << plato.id_instancia << "del pedido" << idPedido << "devuelto a cocina.";
      break;
    }
  }
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
}

QJsonObject LogicaNegocio::getEstadoParaManagerYRanking() {
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
  // Lógica de ranking (simplificada)
  for(const auto& par : m_conteoPlatosRanking){
      QJsonObject item;
      item["nombre"] = QString::fromStdString(m_menu.at(par.first).nombre);
      item["cantidad"] = par.second;
      rankingArray.append(item);
  }
  data["ranking"] = rankingArray;

  mensaje[Protocolo::EVENTO] = Protocolo::ACTUALIZACION_ESTADO_GENERAL;
  mensaje[Protocolo::DATA] = data;
 
  return mensaje;
}

QJsonObject LogicaNegocio::getEstadoParaEstacion(const std::string& nombreEstacion) {
  QJsonObject mensaje;
  QJsonObject data;
  QJsonArray platosPendientes;

  // Verificamos si la estación existe en nuestro mapa de colas
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

void LogicaNegocio::notificarManagersYRanking() {
  QJsonObject estado = getEstadoParaManagerYRanking();
  for (ManejadorCliente* manejador : m_manejadoresActivos) {
    QString rol = manejador->getRol();
    if (rol == "ManagerChef" || rol == "Ranking") {
      emit enviarRespuesta(manejador, estado);
    }
  }
}

void LogicaNegocio::notificarEstacion(const std::string& nombreEstacion) {
  QJsonObject estadoEstacion = getEstadoParaEstacion(nombreEstacion);
  
  if (estadoEstacion.isEmpty()) return;

  for (ManejadorCliente* manejador : m_manejadoresActivos) {
    // Asumimos que la estación se identifica con su nombre en el campo 'rol'
    // o en un campo adicional. Para simplificar, comparamos el nombre de la estación
    // con un identificador del cliente.
    // NOTA: Esto requiere que el cliente Estación se identifique con su nombre.
    // Por ejemplo: { "comando": "IDENTIFICARSE", "rol": "EstacionCocina", "nombre_estacion": "Carnes" }
    // Asumiremos que el 'nombre' se guarda en el 'rol' para este ejemplo.
    if (manejador->getRol().toStdString() == nombreEstacion) {
      emit enviarRespuesta(manejador, estadoEstacion);
      qDebug() << "Notificando a la estación:" << QString::fromStdString(nombreEstacion);
    }
  }
}

void LogicaNegocio::notificarRecepcionista(int idRecepcionista, long long idPedido) {
  QJsonObject notificacion;
  notificacion[Protocolo::EVENTO] = "PLATO_LISTO";
  QJsonObject data;
  data["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(idPedido));
  if (m_pedidosActivos.count(idPedido)) {
      data["mesa"] = m_pedidosActivos.at(idPedido).numero_mesa;
  }
  notificacion[Protocolo::DATA] = data;

  for (ManejadorCliente* manejador : m_manejadoresActivos) {
    if (manejador->getRol() == "Recepcionista" && manejador->getIdActor() == idRecepcionista) {
      emit enviarRespuesta(manejador, notificacion);
      qDebug() << "Notificando a recepcionista" << idRecepcionista << "que el pedido" << idPedido << "tiene platos listos.";
      break; 
    }
  }
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



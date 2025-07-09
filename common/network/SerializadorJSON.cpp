#include "SerializadorJSON.h"
#include <QJsonArray>
#include <QDebug>

QJsonObject SerializadorJSON::platoDefinicionToJson(const PlatoDefinicion& plato) {
  QJsonObject json;
  json["id"] = plato.id;
  json["nombre"] = QString::fromStdString(plato.nombre);
  json["costo"] = plato.costo;
  json["tiempo_preparacion_estimado"] = plato.tiempo_preparacion_estimado;
  json["estacion"] = QString::fromStdString(plato.estacion);
  return json;
}

PlatoDefinicion SerializadorJSON::jsonToPlatoDefinicion(const QJsonObject& json) {
  PlatoDefinicion plato;
  plato.id = json["id"].toInt();
  plato.nombre = json["nombre"].toString().toStdString();
  plato.costo = json["costo"].toDouble();
  plato.tiempo_preparacion_estimado = json["tiempo_preparacion_estimado"].toInt();
  plato.estacion = json["estacion"].toString().toStdString();
  return plato;
}

QJsonObject SerializadorJSON::platoInstanciaToJson(const PlatoInstancia& plato) {
  QJsonObject json;
  json["id_instancia"] = QJsonValue::fromVariant(QVariant::fromValue(plato.id_instancia));
  json["id_plato_definicion"] = plato.id_plato_definicion;
  json["estado"] = estadoPlatoToString(plato.estado);
  // Timestamps pueden ser convertidos a string ISO 8601 si es necesario
  return json;
}

PlatoInstancia SerializadorJSON::jsonToPlatoInstancia(const QJsonObject& json) {
  PlatoInstancia plato;
  plato.id_instancia = json["id_instancia"].toVariant().toLongLong();
  plato.id_plato_definicion = json["id_plato_definicion"].toInt();
  plato.estado = stringToEstadoPlato(json["estado"].toString());
  return plato;
}

QJsonObject SerializadorJSON::pedidoMesaToJson(const PedidoMesa& pedido) {
  QJsonObject json;
  json["id_pedido"] = QJsonValue::fromVariant(QVariant::fromValue(pedido.id_pedido));
  json["numero_mesa"] = pedido.numero_mesa;
  json["id_recepcionista"] = pedido.id_recepcionista;
  json["estado_general"] = estadoPedidoToString(pedido.estado_general);
  
  QJsonArray platosArray;
  for (const auto& plato : pedido.platos) {
    platosArray.append(platoInstanciaToJson(plato));
  }
  json["platos"] = platosArray;
  
  return json;
}

PedidoMesa SerializadorJSON::jsonToPedidoMesa(const QJsonObject& json) {
  PedidoMesa pedido;
  pedido.id_pedido = json["id_pedido"].toVariant().toLongLong();
  pedido.numero_mesa = json["numero_mesa"].toInt();
  pedido.id_recepcionista = json["id_recepcionista"].toInt();
  pedido.estado_general = stringToEstadoPedido(json["estado_general"].toString());
  
  QJsonArray platosArray = json["platos"].toArray();
  for (const QJsonValue& val : platosArray) {
    pedido.platos.push_back(jsonToPlatoInstancia(val.toObject()));
  }

  return pedido;
}

// Implementación de helpers para Enums
QString SerializadorJSON::estadoPlatoToString(EstadoPlato estado) {
  switch(estado) {
    case EstadoPlato::EN_ESPERA: return "EN_ESPERA";
    case EstadoPlato::EN_PROGRESO: return "EN_PROGRESO";
    case EstadoPlato::FINALIZADO: return "FINALIZADO";
    case EstadoPlato::CANCELADO: return "CANCELADO";
    case EstadoPlato::ENTREGADO: return "ENTREGADO";
    default: return "DESCONOCIDO";
  }
}

EstadoPlato SerializadorJSON::stringToEstadoPlato(const QString& str) {
  if (str == "EN_ESPERA") return EstadoPlato::EN_ESPERA;
  if (str == "EN_PROGRESO") return EstadoPlato::EN_PROGRESO;
  if (str == "FINALIZADO") return EstadoPlato::FINALIZADO;
  if (str == "CANCELADO") return EstadoPlato::CANCELADO;
  if (str == "ENTREGADO") return EstadoPlato::ENTREGADO;
  return EstadoPlato::EN_ESPERA; // Default
}

QString SerializadorJSON::estadoPedidoToString(EstadoPedido estado) {
  switch(estado) {
    case EstadoPedido::PENDIENTE: return "PENDIENTE";
    case EstadoPedido::EN_PROGRESO: return "EN_PROGRESO";
    case EstadoPedido::COMPLETADO: return "COMPLETADO";
    case EstadoPedido::CANCELADO: return "CANCELADO";
    default: return "DESCONOCIDO";
  }
}

EstadoPedido SerializadorJSON::stringToEstadoPedido(const QString& str) {
  if (str == "PENDIENTE") return EstadoPedido::PENDIENTE;
  if (str == "EN_PROGRESO") return EstadoPedido::EN_PROGRESO;
  if (str == "COMPLETADO") return EstadoPedido::COMPLETADO;
  if (str == "CANCELADO") return EstadoPedido::CANCELADO;
  return EstadoPedido::PENDIENTE; // Default
}

QJsonObject SerializadorJSON::infoPlatoPrioridadToJson(const InfoPlatoPrioridad& info) {
  QJsonObject json;
  json["id_pedido"] = QJsonValue::fromVariant(info.id_pedido);
  json["id_instancia"] = QJsonValue::fromVariant(info.id_instancia_plato);
  json["score"] = info.score_prioridad;
  return json;
}

InfoPlatoPrioridad SerializadorJSON::jsonToInfoPlatoPrioridad(const QJsonObject& json) {
  InfoPlatoPrioridad info;
  info.id_pedido = json["id_pedido"].toVariant().toLongLong();
  info.id_instancia_plato = json["id_instancia"].toVariant().toLongLong();
  info.score_prioridad = json["score"].toDouble();
  return info;
}


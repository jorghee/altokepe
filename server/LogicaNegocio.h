#ifndef LOGICANEGOCIO_H
#define LOGICANEGOCIO_H

#include <QObject>
#include <queue>
#include <unordered_map>
#include <string>
#include <mutex>
#include <vector>
#include <functional>
#include "common/models/PedidoMesa.h"
#include "common/models/PlatoDefinicion.h"
#include "common/models/InfoPlatoPrioridad.h"

class ManejadorCliente;

using ColaPrioridadPlatos = std::priority_queue<InfoPlatoPrioridad, std::vector<InfoPlatoPrioridad>,
      std::greater<InfoPlatoPrioridad>>;

class LogicaNegocio : public QObject {
  Q_OBJECT

private:
  LogicaNegocio(QObject* parent = nullptr);
  static LogicaNegocio* s_instance;

public:
  static LogicaNegocio* instance();

  LogicaNegocio(const LogicaNegocio&) = delete;
  void operator=(const LogicaNegocio&) = delete;
  
  void procesarMensaje(const QJsonObject& mensaje, ManejadorCliente* remitente);
  void simularRecepcionDePedidos();
  void cargarMenuDesdeArchivo(const QString& rutaArchivo);
  void registrarManejador(ManejadorCliente* manejador);
  void eliminarManejador(ManejadorCliente* manejador);

signals:
  void enviarRespuesta(ManejadorCliente* cliente, const QJsonObject& mensaje);

private:
  void procesarNuevoPedido(const QJsonObject& data, ManejadorCliente* remitente);
  void procesarPrepararPedido(const QJsonObject& data, ManejadorCliente* remitente);
  void procesarCancelarPedido(const QJsonObject& data, ManejadorCliente* remitente);
  void procesarConfirmarEntrega(const QJsonObject& data, ManejadorCliente* remitente);
  void procesarDevolverPlato(const QJsonObject& data, ManejadorCliente* remitente); 

  void clasificarPedidos(
    std::vector<PedidoMesa>& pendientes,
    std::vector<PedidoMesa>& enProgreso,
    std::vector<PedidoMesa>& terminados
  );
  QJsonObject getEstadoCompleto(bool incluirMenu = false);
  void notificarActualizacionGeneral();

  std::mutex m_mutex;
  std::vector<ManejadorCliente*> m_manejadoresActivos;
  
  std::unordered_map<int, PlatoDefinicion> m_menu;
  std::unordered_map<long long, PedidoMesa> m_pedidosActivos;
  std::queue<long long> m_colaManagerChef;
  std::unordered_map<std::string, ColaPrioridadPlatos> m_colasPorEstacion;
  std::unordered_map<int, int> m_conteoPlatosRanking;

  long long m_siguienteIdPedido;
  long long m_siguienteIdInstanciaPlato;
};

#endif

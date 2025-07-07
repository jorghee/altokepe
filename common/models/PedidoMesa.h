#ifndef PEDIDOMESA_H
#define PEDIDOMESA_H

#include "PlatoInstancia.h"
#include "Estados.h"
#include <vector>
#include <string>
#include <chrono>

// Representa el pedido completo de una mesa.
struct PedidoMesa {
  long long id_pedido;
  int numero_mesa;
  int id_recepcionista;

  EstadoPedido estado_general;
  std::chrono::system_clock::time_point timestamp_creacion;

  // Contiene todas las instancias de platos para este pedido.
  std::vector<PlatoInstancia> platos;
};

#endif

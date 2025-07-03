#ifndef ESTADOS_H
#define ESTADOS_H

// Estado de un plato individual en el flujo de la cocina.
enum class EstadoPlato {
  EN_ESPERA,
  EN_PROGRESO,
  FINALIZADO,
  CANCELADO,
  ENTREGADO
};

// Estado general de un pedido completo.
enum class EstadoPedido {
  ACTIVO,
  COMPLETADO,
  CANCELADO
};

#endif

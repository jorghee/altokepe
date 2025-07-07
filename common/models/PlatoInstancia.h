#ifndef PLATOINSTANCIA_H
#define PLATOINSTANCIA_H

#include "Estados.h"
#include <chrono>

// Representa un plato específico que fue ordenado.
struct PlatoInstancia {
  long long id_instancia; // ID único para ESTE plato en ESTE pedido
  int id_plato_definicion; // "Foreign Key" al PlatoDefinicion en el menú
  
  EstadoPlato estado;
  std::chrono::system_clock::time_point timestamp_creacion;
  std::chrono::system_clock::time_point timestamp_ultimo_cambio;
};

#endif

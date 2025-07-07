#ifndef PLATODEFINICION_H
#define PLATODEFINICION_H

#include <string>

// Representa la definición de un plato en el menú.
// Es información de "solo lectura" una vez que el servidor arranca.
struct PlatoDefinicion {
  int id;
  std::string nombre;
  double costo;
  int tiempo_preparacion_estimado; // en minutos
  std::string estacion;
};

#endif

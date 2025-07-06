#ifndef RANKING_H
#define RANKING_H

#include <string>
#include <vector>
#include <map>

struct Plato {
    int id;
    std::string nombre;
    int tiempoPreparacion;
};

struct Pedido {
    int idPedido;
    std::vector<Plato> platos;
};

struct RegistroPedido {
    int idPlato;
    std::string nombrePlato;
    int cantidadVendida = 0;
};

// 👇 Importante que esto esté aquí si es variable global
extern std::map<int, RegistroPedido> ranking;

void registrarVenta(const Pedido& pedido);

#endif // RANKING_H

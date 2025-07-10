#pragma once
#include "ArbolBMas.h"
#include <QString>

class GestorPedidos {
public:
    void registrar(const RegistroPedido &pedido);
    RegistroPedido* consultar(int id);
    QList<RegistroPedido> ultimos(int n);
    void cargarDesdeHistorial();  // ✅ cargar desde JSON
    int obtenerUltimoIdPedido() const;  // ✅ ahora correctamente dentro de la clase

private:
    ArbolBMas arbol;
    void guardarFacturaEnArchivo(const RegistroPedido &pedido);
};

extern GestorPedidos gestorPedidos;  // ✅ Singleton global

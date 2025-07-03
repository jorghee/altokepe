#ifndef TARJETAPEDIDO_H
#define TARJETAPEDIDO_H

#include <QWidget>
#include <vector>

class QVBoxLayout;
class TarjetaPlato;

class TarjetaPedido : public QWidget {
  Q_OBJECT
public:
  // Usaremos un identificador para saber a qué pedido nos referimos
  explicit TarjetaPedido(long long idPedido, const QString& titulo, QWidget* parent = nullptr);
  
  void agregarPlato(const QString& nombrePlato, const QString& estado);
  void agregarAcciones(bool esPrimerPedido); // Para agregar botones solo al primero de la cola "Pendiente"
  void agregarAccionesTerminado();
  
signals:
  void prepararPedido(long long idPedido);
  void cancelarPedido(long long idPedido);
  void enviarPedido(long long idPedido);
  void rechazarPedido(long long idPedido);

private:
  long long m_idPedido;
  QVBoxLayout* m_layoutPlatos;
  std::vector<TarjetaPlato*> m_platos;
};

#endif

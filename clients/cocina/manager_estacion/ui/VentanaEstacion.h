#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QString>

class VentanaEstacion : public QWidget {
    Q_OBJECT
public:
    struct InfoPlatoVisual {
        QString nombrePlato;
        double prioridad;
        QString estado;
        long long id_pedido;
        long long id_instancia;
    };

    explicit VentanaEstacion(const QString& nombreEstacion, QWidget* parent = nullptr);

    void cargarPlatosIniciales(const std::vector<InfoPlatoVisual>& platos);
    void agregarNuevoPlato(const InfoPlatoVisual& plato);
    void actualizarEstadoPlato(long long idPedido, long long idInstancia, const QString& nuevoEstado);
    void eliminarPlato(long long idPedido, long long idInstancia);
    
signals:
    void marcarListoSolicitado(long long idPedido, long long idInstancia);

private:
    QTableWidget* tabla;
    QString m_estacion;

    struct PlatoKey {
        long long idPedido;
        long long idInstancia;
    };
    std::vector<PlatoKey> filaToPlatoKey;
};

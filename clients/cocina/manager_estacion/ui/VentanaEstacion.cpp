#include "VentanaEstacion.h"
#include <QHeaderView>
#include <QPushButton>
#include <QDebug>

// Definimos roles para IDs
constexpr int Role_IdPedido = Qt::UserRole + 1;
constexpr int Role_IdInstancia = Qt::UserRole + 2;

VentanaEstacion::VentanaEstacion(const QString& nombreEstacion, QWidget* parent)
    : QWidget(parent), m_estacion(nombreEstacion) {

    setWindowTitle("Estación: " + nombreEstacion);

    tabla = new QTableWidget(0, 4, this);
    tabla->setHorizontalHeaderLabels({"Plato", "Prioridad", "Estado", "Listo"});
    tabla->horizontalHeader()->setStretchLastSection(true);
    tabla->verticalHeader()->setVisible(false);
    tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(tabla);
    setLayout(layout);
}

void VentanaEstacion::cargarPlatosIniciales(const std::vector<InfoPlatoVisual>& platos) {
    tabla->setRowCount(0);
    for (const auto& plato : platos) {
        agregarNuevoPlato(plato);
    }
}

void VentanaEstacion::agregarNuevoPlato(const InfoPlatoVisual& plato) {
    int row = tabla->rowCount();
    tabla->insertRow(row);

    auto* itemNombre = new QTableWidgetItem(plato.nombrePlato);
    itemNombre->setData(Role_IdPedido, QVariant::fromValue(plato.id_pedido));
    itemNombre->setData(Role_IdInstancia, QVariant::fromValue(plato.id_instancia));

    tabla->setItem(row, 0, itemNombre);
    tabla->setItem(row, 1, new QTableWidgetItem(QString::number(plato.prioridad)));
    tabla->setItem(row, 2, new QTableWidgetItem(plato.estado));

    auto* btnListo = new QPushButton("Listo");
    connect(btnListo, &QPushButton::clicked, this, [this, row]() {
        auto* item = tabla->item(row, 0);
        long long idPedido = item->data(Role_IdPedido).toLongLong();
        long long idInstancia = item->data(Role_IdInstancia).toLongLong();
        emit marcarListoSolicitado(idPedido, idInstancia);
    });

    tabla->setCellWidget(row, 3, btnListo);
}

void VentanaEstacion::actualizarEstadoPlato(long long idPedido, long long idInstancia, const QString& nuevoEstado) {
    for (int fila = 0; fila < tabla->rowCount(); ++fila) {
        auto* item = tabla->item(fila, 0);
        if (!item) continue;

        if (item->data(Role_IdPedido).toLongLong() == idPedido &&
            item->data(Role_IdInstancia).toLongLong() == idInstancia) {
            tabla->item(fila, 2)->setText(nuevoEstado);

            if (nuevoEstado.contains("FINALIZADO", Qt::CaseInsensitive)) {
                tabla->removeRow(fila);
            }

            break;
        }
    }
}

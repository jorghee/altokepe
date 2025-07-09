#include "VentanaEstacion.h"
#include <QHeaderView>
#include <QPushButton>
#include <QDebug>

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
    filaToPlatoKey.clear();

    for (const auto& plato : platos) {
        agregarNuevoPlato(plato);
    }
}

void VentanaEstacion::agregarNuevoPlato(const InfoPlatoVisual& plato) {
    int row = tabla->rowCount();
    tabla->insertRow(row);

    tabla->setItem(row, 0, new QTableWidgetItem(plato.nombrePlato));
    tabla->setItem(row, 1, new QTableWidgetItem(QString::number(plato.prioridad)));
    tabla->setItem(row, 2, new QTableWidgetItem(plato.estado));

    auto* btnListo = new QPushButton("Listo");
    connect(btnListo, &QPushButton::clicked, this, [this, row]() {
        if (row < filaToPlatoKey.size()) {
            auto p = filaToPlatoKey[row];
            emit marcarListoSolicitado(p.idPedido, p.idInstancia);
        }
    });

    tabla->setCellWidget(row, 3, btnListo);

    filaToPlatoKey.push_back({plato.id_pedido, plato.id_instancia});
}

void VentanaEstacion::actualizarEstadoPlato(long long idPedido, long long idInstancia, const QString& nuevoEstado) {
    for (int i = 0; i < filaToPlatoKey.size(); ++i) {
        const auto& p = filaToPlatoKey[i];
        if (p.idPedido == idPedido && p.idInstancia == idInstancia) {
            tabla->item(i, 2)->setText(nuevoEstado);
            break;
        }
    }
}

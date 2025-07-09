#include "VentanaRanking.h"
#include <QVBoxLayout>
#include <QHeaderView>

VentanaRanking::VentanaRanking(QWidget* parent) : QWidget(parent) {
    setWindowTitle("Ranking de Platos Vendidos");
    resize(400, 500);

    m_tabla = new QTableWidget(this);
    m_tabla->setColumnCount(2);
    m_tabla->setHorizontalHeaderLabels({ "Plato", "Cantidad Vendida" });
    m_tabla->horizontalHeader()->setStretchLastSection(true);
    m_tabla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tabla->setSelectionMode(QAbstractItemView::NoSelection);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_tabla);
    setLayout(layout);

    m_cliente = new ClienteRanking(this);
    connect(m_cliente, &ClienteRanking::rankingActualizado, this, &VentanaRanking::actualizarRanking);
    m_cliente->conectar("127.0.0.1", 4242); // Ajusta el puerto si usas otro
}

void VentanaRanking::actualizarRanking(const QJsonArray& ranking) {
    m_tabla->clearContents();
    m_tabla->setRowCount(ranking.size());

    for (int i = 0; i < ranking.size(); ++i) {
        QJsonObject item = ranking[i].toObject();
        QString nombre = item.value("nombre").toString();
        int cantidad = item.value("cantidad").toInt();

        m_tabla->setItem(i, 0, new QTableWidgetItem(nombre));
        m_tabla->setItem(i, 1, new QTableWidgetItem(QString::number(cantidad)));
    }
}

#include "RankingWindow.h"
#include <QVBoxLayout>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QTimer>
#include <QLabel>
#include <QFont>
#include <algorithm>
#include <cstdlib>
#include <ctime>

RankingWindow::RankingWindow(QWidget *parent)
    : QWidget(parent) {
    // Pantalla estilo presentación elegante
    setWindowTitle("Ranking de Platos - Bienvenido");
    resize(900, 600);
    setStyleSheet("background-color: #fdfcf8;");

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titulo = new QLabel("🥇 Ranking de Platos Más Vendidos");
    titulo->setAlignment(Qt::AlignCenter);
    QFont font("Segoe UI", 24, QFont::Bold);
    titulo->setFont(font);
    titulo->setStyleSheet("color: #444; margin: 10px;");

    tablaRanking = new QTableWidget(this);
    tablaRanking->setColumnCount(2);
    tablaRanking->setHorizontalHeaderLabels({"Plato", "Ventas"});
    tablaRanking->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablaRanking->verticalHeader()->setVisible(false);
    tablaRanking->setStyleSheet(
        "QTableWidget { font-size: 18px; background-color: #fff; border-radius: 15px; }"
        "QHeaderView::section { background-color: #f5f5f5; padding: 10px; border: none; font-size: 16px; }"
        "QTableWidget::item { padding: 10px; }"
        );
    tablaRanking->setAlternatingRowColors(true);
    tablaRanking->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(titulo);
    layout->addWidget(tablaRanking);
    setLayout(layout);

    // Simular datos al inicio
    simularRanking();

    // Actualizar cada 10 segundos como si fueran nuevas ventas
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &RankingWindow::simularRanking);
    timer->start(10000);
}

void RankingWindow::simularRanking() {
    // Simular algunos platos
    std::vector<RegistroPedido> lista = {
        {1, "Lomo Saltado", rand() % 50 + 50},
        {2, "Pollo a la Brasa", rand() % 50 + 30},
        {3, "Ceviche", rand() % 30 + 20},
        {4, "Tallarines Verdes", rand() % 20 + 10},
        {5, "Arroz con Pollo", rand() % 25 + 15}
    };

    std::sort(lista.begin(), lista.end(), [](const RegistroPedido& a, const RegistroPedido& b) {
        return a.cantidadVendida > b.cantidadVendida;
    });

    tablaRanking->setRowCount(static_cast<int>(lista.size()));

    for (int i = 0; i < lista.size(); ++i) {
        tablaRanking->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(lista[i].nombrePlato)));
        tablaRanking->setItem(i, 1, new QTableWidgetItem(QString::number(lista[i].cantidadVendida)));
    }
}

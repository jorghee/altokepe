#include "RankingWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QFrame>
#include <QTimer>
#include <QSpacerItem>
#include <algorithm>
#include <cstdlib>

// 🎨 Paleta tipo CSS
const QString COLOR_PRIMARY = "#2C3E50";
const QString COLOR_ACCENT = "#3498db";
const QString COLOR_BG = "#f0f4f8";
const QString COLOR_TOP1 = "#D4AF37";
const QString COLOR_ROW_BG = "#ffffff";
const QString COLOR_BORDER = "#cccccc";

RankingWindow::RankingWindow(QWidget *parent)
    : QWidget(parent) {
    setWindowTitle("Clasificación de Platos");
    resize(1000, 700);
    setStyleSheet("background-color: " + COLOR_BG + ";");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 🧢 Encabezado tipo presentación
    QLabel* titulo = new QLabel("✨ CLASIFICACIÓN FINAL ✨\nRESTAURANTE ALTA GASTRONOMÍA");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setStyleSheet("font-family: 'Georgia'; font-size: 28px; font-weight: bold; color: " + COLOR_PRIMARY + "; margin-top: 20px; margin-bottom: 30px;");
    mainLayout->addWidget(titulo);

    contenedorRanking = new QVBoxLayout();
    contenedorRanking->setSpacing(20);
    contenedorRanking->setContentsMargins(50, 10, 50, 30);

    mainLayout->addLayout(contenedorRanking);

    // ⏱️ Simular actualización cada 5 segundos
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &RankingWindow::simularRanking);
    timer->start(5000);

    simularRanking();
}

void RankingWindow::simularRanking() {
    // 🥘 Datos simulados
    std::vector<RegistroPedido> lista = {
        {1, "Lomo Saltado", rand() % 120 + 100},
        {2, "Pollo a la Brasa", rand() % 100 + 80},
        {3, "Ceviche", rand() % 90 + 60},
        {4, "Tallarines Verdes", rand() % 80 + 50},
        {5, "Arroz con Pollo", rand() % 70 + 40}
    };

    std::sort(lista.begin(), lista.end(), [](const RegistroPedido& a, const RegistroPedido& b) {
        return a.cantidadVendida > b.cantidadVendida;
    });

    int maxVentas = lista.front().cantidadVendida;

    // 🧹 Limpiar vista anterior
    QLayoutItem* item;
    while ((item = contenedorRanking->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // 🔁 Generar filas visuales
    for (int i = 0; i < lista.size(); ++i) {
        const auto& r = lista[i];

        QFrame* fila = new QFrame();
        fila->setStyleSheet("background-color: " + COLOR_ROW_BG + "; border: 2px solid " + COLOR_BORDER + "; border-radius: 15px;");
        fila->setFixedHeight(80);

        QHBoxLayout* filaLayout = new QHBoxLayout(fila);
        filaLayout->setContentsMargins(20, 10, 20, 10);

        // 🥇 Posición
        QLabel* lblPos = new QLabel(QString::number(i + 1));
        lblPos->setFixedWidth(40);
        lblPos->setAlignment(Qt::AlignCenter);
        lblPos->setStyleSheet("font-size: 24px; font-weight: bold; color: " + (i == 0 ? COLOR_TOP1 : COLOR_PRIMARY) + ";");
        filaLayout->addWidget(lblPos);

        // 📛 Nombre del plato
        QLabel* lblNombre = new QLabel(QString::fromStdString(r.nombrePlato));
        lblNombre->setStyleSheet("font-size: 20px; font-weight: bold; color: " + COLOR_PRIMARY + ";");
        filaLayout->addWidget(lblNombre, 2);

        // 📊 Barra de popularidad
        QProgressBar* barra = new QProgressBar();
        barra->setMinimum(0);
        barra->setMaximum(100);
        barra->setValue(static_cast<int>((r.cantidadVendida * 100) / maxVentas));
        barra->setTextVisible(false);
        barra->setFixedHeight(20);
        barra->setStyleSheet(
            "QProgressBar { background-color: #e6e6e6; border-radius: 10px; }"
            "QProgressBar::chunk { background-color: " + (i == 0 ? COLOR_TOP1 : COLOR_ACCENT) + "; border-radius: 10px; }"
            );
        filaLayout->addWidget(barra, 4);

        // 🔢 Ventas
        QLabel* lblVentas = new QLabel(QString::number(r.cantidadVendida));
        lblVentas->setAlignment(Qt::AlignCenter);
        lblVentas->setStyleSheet("font-size: 22px; font-weight: bold; color: " + COLOR_PRIMARY + ";");
        lblVentas->setFixedWidth(60);
        filaLayout->addWidget(lblVentas);

        contenedorRanking->addWidget(fila);
    }
}

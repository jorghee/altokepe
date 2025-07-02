#ifndef RANKINGWINDOW_H
#define RANKINGWINDOW_H

#include <QWidget>
#include <QTableWidget>
#include "../Ranking/Ranking.h" // o "../Ranking/Ranking.h" según tu ruta

class RankingWindow : public QWidget {
    Q_OBJECT

public:
    explicit RankingWindow(QWidget *parent = nullptr);

private:
    QTableWidget* tablaRanking;
    void simularRanking();  // reemplaza actualizarRanking
};

#endif // RANKINGWINDOW_H

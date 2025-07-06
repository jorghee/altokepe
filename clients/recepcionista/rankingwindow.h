#ifndef RANKINGWINDOW_H
#define RANKINGWINDOW_H

#include <QWidget>
#include <QVBoxLayout>

struct RegistroPedido {
    int idPlato;
    std::string nombrePlato;
    int cantidadVendida;
};

class RankingWindow : public QWidget {
    Q_OBJECT

public:
    explicit RankingWindow(QWidget *parent = nullptr);

private:
    QVBoxLayout* contenedorRanking;
    void simularRanking();
};

#endif // RANKINGWINDOW_H

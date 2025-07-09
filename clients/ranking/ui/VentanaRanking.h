#ifndef VENTANARANKING_H
#define VENTANARANKING_H

#include <QWidget>
#include <QTableWidget>
#include "../network/ClienteRanking.h"

class VentanaRanking : public QWidget {
    Q_OBJECT
public:
    explicit VentanaRanking(QWidget* parent = nullptr);

private slots:
    void actualizarRanking(const QJsonArray& ranking);

private:
    QTableWidget* m_tabla;
    ClienteRanking* m_cliente;
};

#endif // VENTANARANKING_H

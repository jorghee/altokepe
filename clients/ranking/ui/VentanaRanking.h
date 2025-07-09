#ifndef VENTANARANKING_H
#define VENTANARANKING_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QJsonArray>

class VentanaRanking : public QWidget {
    Q_OBJECT
public:
    explicit VentanaRanking(QWidget* parent = nullptr);
    void actualizarRanking(const QJsonArray& ranking);

private:
    QTableWidget* tablaRanking;
};

#endif // VENTANARANKING_H

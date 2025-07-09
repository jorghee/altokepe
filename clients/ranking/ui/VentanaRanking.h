#ifndef VENTANARANKING_H
#define VENTANARANKING_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QJsonArray>

//cambios
#include <QScrollArea>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

class VentanaRanking : public QWidget {
    Q_OBJECT
public:
    explicit VentanaRanking(QWidget* parent = nullptr);
    void actualizarRanking(const QJsonArray& ranking);

private slots:
    void mostrarMenuAgrupado(const QJsonArray& menu); // cambio

private:
    QTableWidget* tablaRanking;

    QJsonArray m_menu; // cambio
    //cambio
    QScrollArea* scrollMenu;
    QWidget* menuContainer;
    QGridLayout* gridMenuLayout;
};

#endif // VENTANARANKING_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QComboBox>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private:
    QWidget *centralWidget;

    QLineEdit *pedidoInput;
    QLineEdit *mesaInput;
    QLineEdit *personasInput;

    QComboBox *comboPlatos;
    QTableWidget *tablaPedidos;
    QLabel *labelTotal; // ← agregado para mostrar el total del pedido
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QSpinBox>
#include <QMessageBox>
#include <QDebug>
#include <QMap>

struct Plato {
    QString nombre;
    double precio;
    int tiempoPreparacion;
    QString categoria;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void mesaClicked(int numeroMesa);
    void agregarPlato();
    void enviarPedido();
    void nuevoPedido();

private:

    static const int NUMERO_MESAS = 10;
    QMap<QString, Plato> mapaPlatos;

    void setupUI();
    void setupMesasSection();
    void setupPedidoSection();
    void crearBotonesMesas();
    void connectSignals();
    void aplicarEstilos();
    void cargarMenu();

    void seleccionarMesa(int numeroMesa);
    void actualizarTotalGeneral();
    void limpiarFormulario();
    bool verificarDesocuparMesa(int numeroMesa);
    void actualizarEstiloMesa(int numeroMesa, bool ocupada);

    QWidget *centralWidget;
    QWidget *mesasWidget;
    QWidget *pedidoWidget;

    QGridLayout *mesasLayout;
    QList<QPushButton*> botonesMesas;


    QLabel *pedidoLabel;
    QLabel *mesaSeleccionadaLabel;
    QComboBox *comboPlatos;
    QLineEdit *descripcionEdit;  
    QPushButton *btnAgregarPlato;
    QPushButton *btnEnviarPedido;
    QPushButton *btnNuevoPedido;
    QTableWidget *tablaPedidos;
    QLabel *labelTotal;

    // Variables de estado
    bool estadoMesas[NUMERO_MESAS];
    int contadorPedidos;
    int mesaActual;
};

#endif 

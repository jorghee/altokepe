    #include "PanelPedido.h"
    #include <QVBoxLayout>
    #include <QHBoxLayout>
    #include <QLabel>
    #include <QLineEdit>
    #include <QListWidget>
    #include <QTableWidget>
    #include <QPushButton>
    #include <QSpinBox>
    #include <QMessageBox>
    #include <QJsonObject>

    PanelPedido::PanelPedido(QWidget *parent)
        : QWidget(parent) {
        cliente.conectarAlServidor("127.0.0.1", 5555);
        configurarUI();

        connect(&cliente, &ClienteRecepcionista::menuActualizado,
                this, &PanelPedido::actualizarMenu);
    }

    void PanelPedido::configurarUI() {
        auto *layout = new QVBoxLayout(this);

        labelMesa = new QLabel("Mesa: --", this);
        labelPedido = new QLabel(this);
        numeroPedido = gestorPedidos.obtenerUltimoIdPedido() + 1;
        labelPedido->setText(QString("Pedido N°: %1").arg(QString("%1").arg(numeroPedido, 4, 10, QChar('0'))));


        auto *nombreLayout = new QHBoxLayout();
        auto *labelNombre = new QLabel("Nombre del cliente:", this);
        inputNombre = new QLineEdit(this);
        nombreLayout->addWidget(labelNombre);
        nombreLayout->addWidget(inputNombre);

        auto *tituloPlatos = new QLabel("Platos:", this);
        tituloPlatos->setStyleSheet("font-weight: bold;");

        listaPlatos = new QListWidget(this);
        listaPlatos->setFixedHeight(170);
        listaPlatos->setStyleSheet("padding-left: 6px;");

        connect(listaPlatos, &QListWidget::itemClicked, this, [=](QListWidgetItem *item) {
            int id = item->data(Qt::UserRole).toInt();
            QString nombre = item->text();
            agregarPlatoAlPedido(id, nombre);
        });

        tablaPedido = new QTableWidget(0, 4, this);
        tablaPedido->setHorizontalHeaderLabels({"Plato", "Cantidad", "Precio", "Acción"});
        tablaPedido->horizontalHeader()->setStretchLastSection(true);
        tablaPedido->setColumnWidth(0, 200);
        tablaPedido->setColumnWidth(1, 80);
        tablaPedido->setColumnWidth(2, 100);
        tablaPedido->setColumnWidth(3, 60);

        labelTotal = new QLabel("Total: S/ 0.00", this);
        labelTotal->setAlignment(Qt::AlignRight);
        labelTotal->setStyleSheet("font-size: 18px; font-weight: bold;");

        botonEnviar = new QPushButton("Enviar Pedido", this);
        botonEnviar->setObjectName("botonEnviar");
        botonEnviar->setStyleSheet("font-weight: bold;");
        connect(botonEnviar, &QPushButton::clicked, this, &PanelPedido::enviarPedido);

        layout->addWidget(labelMesa);
        layout->addWidget(labelPedido);
        layout->addLayout(nombreLayout);
        layout->addSpacing(10);
        layout->addWidget(tituloPlatos);
        layout->addWidget(listaPlatos);
        layout->addSpacing(10);
        layout->addWidget(tablaPedido);
        layout->addWidget(labelTotal);
        layout->addSpacing(10);
        layout->addWidget(botonEnviar);

        setLayout(layout);
    }

    void PanelPedido::setNumeroMesa(int numero) {
        mesaActual = numero;
        labelMesa->setText(QString("Mesa: %1").arg(numero));
    }

    void PanelPedido::actualizarMenu(const QJsonArray &menu) {
        listaPlatos->clear();
        menuPlatos.clear();
        preciosPlatos.clear(); // ✅ importante

        for (const QJsonValue &valor : menu) {
            QJsonObject plato = valor.toObject();

            int id = plato["id"].toInt();
            QString nombre = plato["nombre"].toString();
            double costo = plato["precio"].toDouble(); // ✅ extraer el campo correcto

            auto *item = new QListWidgetItem(nombre);
            item->setData(Qt::UserRole, id);
            listaPlatos->addItem(item);

            menuPlatos[id] = nombre;
            preciosPlatos[id] = costo; // ✅ guardar el precio correctamente
        }
    }


    void PanelPedido::agregarPlatoAlPedido(int idPlato, const QString &nombre) {
        for (int i = 0; i < tablaPedido->rowCount(); ++i) {
            if (tablaPedido->item(i, 0)->data(Qt::UserRole).toInt() == idPlato)
                return;
        }

        int row = tablaPedido->rowCount();
        tablaPedido->insertRow(row);

        auto *item = new QTableWidgetItem(nombre);
        item->setData(Qt::UserRole, idPlato);
        tablaPedido->setItem(row, 0, item);

        auto *spin = new QSpinBox();
        spin->setRange(1, 10);
        tablaPedido->setCellWidget(row, 1, spin);
        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, &PanelPedido::actualizarTotal);

        double precio = preciosPlatos.value(idPlato, 0.0);
        auto *itemPrecio = new QTableWidgetItem(QString("S/ %1").arg(precio, 0, 'f', 2));
        itemPrecio->setData(Qt::UserRole, precio);
        itemPrecio->setTextAlignment(Qt::AlignRight);
        tablaPedido->setItem(row, 2, itemPrecio);

        auto *btnEliminar = new QPushButton("❌", this);
        btnEliminar->setStyleSheet("background-color: white; color: #F44336; border: none;");
        connect(btnEliminar, &QPushButton::clicked, this, [=]() {
            eliminarPlato(row);
        });
        tablaPedido->setCellWidget(row, 3, btnEliminar);

        actualizarTotal();
    }

    void PanelPedido::eliminarPlato(int fila) {
        tablaPedido->removeRow(fila);
        actualizarTotal();
    }

    void PanelPedido::actualizarTotal() {
        double total = 0.0;

        for (int i = 0; i < tablaPedido->rowCount(); ++i) {
            int cantidad = static_cast<QSpinBox *>(tablaPedido->cellWidget(i, 1))->value();
            double precioUnitario = tablaPedido->item(i, 2)->data(Qt::UserRole).toDouble();

            double precioTotal = cantidad * precioUnitario;
            tablaPedido->item(i, 2)->setText(QString("S/ %1").arg(precioTotal, 0, 'f', 2)); // ✅ actualizar celda

            total += precioTotal;
        }

        labelTotal->setText(QString("Total: S/ %1").arg(total, 0, 'f', 2));
    }


    void PanelPedido::enviarPedido() {
        if (mesaActual == -1) {
            QMessageBox::warning(this, "Error", "Debe seleccionar una mesa.");
            return;
        }

        QString nombreCliente = inputNombre->text().trimmed();
        if (nombreCliente.isEmpty()) {
            QMessageBox::warning(this, "Error", "Ingrese el nombre del cliente.");
            return;
        }

        QJsonArray platosJson;
        QList<PlatoPedido> platosLista;
        double total = 0.0;

        for (int i = 0; i < tablaPedido->rowCount(); ++i) {
            int id = tablaPedido->item(i, 0)->data(Qt::UserRole).toInt();
            int cantidad = static_cast<QSpinBox *>(tablaPedido->cellWidget(i, 1))->value();
            double precioUnitario = preciosPlatos.value(id, 0.0);

            if (cantidad >= 1) {
                platosJson.append(QJsonObject{{"id", id}, {"cantidad", cantidad}});
                platosLista.append({id, menuPlatos[id], cantidad, precioUnitario});
                total += cantidad * precioUnitario;
            }
        }

        if (platosJson.isEmpty()) {
            QMessageBox::warning(this, "Error", "Debe agregar al menos un plato.");
            return;
        }

        // Enviar al servidor (lógica ya existente)
        cliente.enviarNuevoPedido(mesaActual, numeroPedido, platosJson);

        // Registrar localmente en árbol B+
        RegistroPedido pedido;
        pedido.idPedido = numeroPedido;
        pedido.nombreCliente = nombreCliente;
        pedido.numeroMesa = mesaActual;
        pedido.platos = platosLista;
        pedido.total = total;

        gestorPedidos.registrar(pedido);

        QMessageBox::information(this, "Enviado", "Pedido enviado correctamente.");

        numeroPedido++;
        labelPedido->setText(QString("Pedido N°: %1").arg(QString("%1").arg(numeroPedido, 4, 10, QChar('0'))));

        inputNombre->clear();
        tablaPedido->setRowCount(0);
        actualizarTotal();

        mesaActual = -1;
        labelMesa->setText("Mesa: --");
    }

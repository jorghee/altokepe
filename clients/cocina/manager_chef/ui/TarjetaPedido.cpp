#include "TarjetaPedido.h"
#include "TarjetaPlato.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

TarjetaPedido::TarjetaPedido(long long idPedido, const QString& titulo, QWidget* parent)
  : QWidget(parent), m_idPedido(idPedido) {

  this->setProperty("class", "TarjetaPedido"); // Para QSS

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  QLabel* tituloLabel = new QLabel(titulo, this);
  tituloLabel->setProperty("class", "TituloPedido");

  mainLayout->addWidget(tituloLabel);
  
  m_layoutPlatos = new QVBoxLayout();
  mainLayout->addLayout(m_layoutPlatos);
  mainLayout->addStretch(); // Empuja los botones hacia abajo si se añaden

  setLayout(mainLayout);
}

void TarjetaPedido::agregarPlato(const QString& nombrePlato, const QString& estado) {
  TarjetaPlato* platoWidget = new TarjetaPlato(nombrePlato, estado, this);
  m_platos.push_back(platoWidget);
  m_layoutPlatos->addWidget(platoWidget);
}

void TarjetaPedido::agregarAcciones(bool esPrimerPedido) {
  if (!esPrimerPedido) return;

  QHBoxLayout* layoutBotones = new QHBoxLayout();
  QPushButton* btnCancelar = new QPushButton("Cancelar", this);
  QPushButton* btnPreparar = new QPushButton("Preparar", this);
  
  btnCancelar->setObjectName("btnCancelar");
  btnPreparar->setObjectName("btnPreparar");

  connect(btnPreparar, &QPushButton::clicked, this, [this](){ emit prepararPedido(m_idPedido); });
  connect(btnCancelar, &QPushButton::clicked, this, [this](){ emit cancelarPedido(m_idPedido); });

  layoutBotones->addStretch();
  layoutBotones->addWidget(btnCancelar);
  layoutBotones->addWidget(btnPreparar);

  static_cast<QVBoxLayout*>(layout())->addLayout(layoutBotones);
}

void TarjetaPedido::agregarAccionesTerminado() {
  QHBoxLayout* layoutBotones = new QHBoxLayout();
  QPushButton* btnRechazar = new QPushButton("Rechazar", this);
  QPushButton* btnEnviar = new QPushButton("Enviar", this);

  btnRechazar->setObjectName("btnRechazar");
  btnEnviar->setObjectName("btnEnviar");

  connect(btnEnviar, &QPushButton::clicked, this, [this]() { emit enviarPedido(m_idPedido); });
  connect(btnRechazar, &QPushButton::clicked, this, [this]() { emit rechazarPedido(m_idPedido); });

  layoutBotones->addStretch();
  layoutBotones->addWidget(btnRechazar);
  layoutBotones->addWidget(btnEnviar);

  static_cast<QVBoxLayout*>(layout())->addLayout(layoutBotones);
}

#include "TarjetaPlato.h"
#include <QHBoxLayout>
#include <QLabel>

TarjetaPlato::TarjetaPlato(const QString& nombrePlato, const QString& estadoInicial, QWidget* parent)
  : QWidget(parent) {
  
  setObjectName("TarjetaPlato");
  
  QHBoxLayout* layout = new QHBoxLayout(this);
  m_nombreLabel = new QLabel(nombrePlato, this);
  m_estadoLabel = new QLabel(estadoInicial, this);
  
  // Estilo para el estado
  m_estadoLabel->setAlignment(Qt::AlignCenter);
  m_estadoLabel->setStyleSheet("background-color: #f0c040; color: #333; border-radius: 8px; padding: 4px; font-weight: bold;");

  layout->addWidget(m_nombreLabel);
  layout->addStretch();
  layout->addWidget(m_estadoLabel);
  
  setLayout(layout);
}

void TarjetaPlato::actualizarEstado(const QString& nuevoEstado) {
  m_estadoLabel->setText(nuevoEstado);
  // TODO: Cambiar el color del label de estado según el nuevo estado
}

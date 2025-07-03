#include "ManejadorCliente.h"
#include "LogicaNegocio.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

ManejadorCliente::ManejadorCliente(qintptr socketDescriptor, QObject* parent)
  : QObject(parent), m_socketDescriptor(socketDescriptor) {
}

ManejadorCliente::~ManejadorCliente() {
  qDebug() << "Manejador de cliente destruido para socket" << m_socketDescriptor;
}

void ManejadorCliente::procesar() {
  m_socket = new QTcpSocket();
  if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
    qCritical() << "No se pudo establecer el descriptor de socket" << m_socketDescriptor;
    emit finished();
    return;
  }

  connect(m_socket, &QTcpSocket::readyRead, this, &ManejadorCliente::listoParaLeer);
  connect(m_socket, &QTcpSocket::disconnected, this, &ManejadorCliente::desconectado);
  qDebug() << "Cliente conectado en socket" << m_socketDescriptor;
}

void ManejadorCliente::listoParaLeer() {
  m_buffer.append(m_socket->readAll());
  procesarBuffer();
}

void ManejadorCliente::procesarBuffer() {
  while (m_buffer.contains('\n')) {
    int newlinePos = m_buffer.indexOf('\n');
    QByteArray jsonData = m_buffer.left(newlinePos);
    m_buffer.remove(0, newlinePos + 1);
    if (jsonData.isEmpty()) continue;
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
      qWarning() << "Mensaje JSON inválido recibido en socket" << m_socketDescriptor;
      continue;
    }
    LogicaNegocio::instance()->procesarMensaje(doc.object());
  }
}

void ManejadorCliente::desconectado() {
  qDebug() << "Cliente desconectado del socket" << m_socketDescriptor;
  m_socket->deleteLater();
  emit finished();
}

void ManejadorCliente::enviarMensaje(const QJsonObject& mensaje) {
  if (m_socket && m_socket->isOpen()) {
    QJsonDocument doc(mensaje); 
    m_socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
  }
}

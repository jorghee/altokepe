#ifndef MANEJADORCLIENTE_H
#define MANEJADORCLIENTE_H

#include <QObject>
#include <QTcpSocket>

class ManejadorCliente : public QObject {
  Q_OBJECT
public:
  explicit ManejadorCliente(qintptr socketDescriptor, QObject* parent = nullptr);
  virtual ~ManejadorCliente();

  QString getRol() const;
  int getIdActor() const;

public slots:
  void procesar();
  void enviarMensaje(const QJsonObject& mensaje);

private slots:
  void listoParaLeer();
  void desconectado();

signals:
  void finished();

private:
  void procesarBuffer();
  void identificarCliente(const QJsonObject& data);

  qintptr m_socketDescriptor;
  QTcpSocket* m_socket = nullptr;
  QByteArray m_buffer;

  QString m_rol;
  int m_idActor = -1; // -1 indica no identificado
};

#endif

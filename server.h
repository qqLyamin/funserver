#ifndef SERVER_H
#define SERVER_H

//Qt includes
#include <QObject>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDateTime>
#include <QThread>

//local includes
#include "database.h"

enum serverStatus {DEAD, ALIVE};

//Q_DECLARE_METATYPE(qint64) //рег нового типа для передачи между потоками

class my_server : public QObject
{

    Q_OBJECT
public:
    QTcpServer qserver;
    bool listenOK = false;

    my_server() {
        connect(&qserver, &QTcpServer::newConnection, this, &my_server::newConnection);
    }

signals:
    void isListening(bool);
    void newConnection();
    void nextPendingConnection(QTcpSocket *);

public slots:
    void listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 17888) {
        qDebug() << "we are listening";
        listenOK = qserver.listen(QHostAddress::Any, 17888);
        emit isListening(listenOK);
    }

    void nextPendingConnectionRequest() {
        qDebug() << "my_server::nextPendingConnectionRequest";
        emit nextPendingConnection(qserver.nextPendingConnection());
    }
};

class client : public QObject
{
    Q_OBJECT
public:
    QTcpSocket * clientSocket;

public:
    client(QTcpSocket * nextPendingConnection) : clientSocket(nextPendingConnection) {
        connect(clientSocket, &QAbstractSocket::readyRead, this, &client::readyReadSlot);
        connect(clientSocket, &QAbstractSocket::disconnected, this, &client::disconnectedSlot);
    }
    ~client() {
        clientSocket->close();
        clientSocket->deleteLater();
    }

signals:
    void readyRead(const QByteArray &,  qint64);
    void disconnected();

public slots:
    void readyReadSlot() {
        QByteArray tmp = clientSocket->readAll();
        emit readyRead(tmp, clientSocket->socketDescriptor());
    }

    void disconnectedSlot() {
        disconnected();
    }

    void write(const QByteArray & data,  qint64 Descriptor) {
        if (Descriptor == clientSocket->socketDescriptor()) {
           clientSocket->write(data);
        }
    }
};

class Server : public QTcpServer
{
    Q_OBJECT

    database * db;
    my_server qserver;

    int _current_user;

    serverStatus status;
    bool authorization = false;

    QThread * listenThread;
    QList<QThread *> clientsThreads;
    QList<client *> clientsList;

public:
    Server();
    ~Server() {
        for (auto client : clientsList) {
            delete client;
        }
        for (auto thread : clientsThreads) {
            delete thread;
        }
        delete db;
        delete listenThread;
    }

public slots:
    void newUser();
    void slotReadyRead(const QByteArray &,  qint64);

private slots:
    void forceClose();
    void disconnectUser();
    void authorizationSuccess();
    void authorizationException();
    void reg_false(qint64);
    void reg_true(qint64);
    void treatNewUser(QTcpSocket * nextPendingConnection);

signals:
    void writeToClient(const QByteArray & data,  qint64 Descriptor);
    void checkPendingConnection();
    void checkUser(const QStringList &);
    void registrate(const QStringList &, qint64);
};

#endif // SERVER_H

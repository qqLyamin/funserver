#ifndef SERVER_H
#define SERVER_H

//Qt includes
#include <QObject>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDateTime>

//local includes
#include "database.h"

enum serverStatus {DEAD, ALIVE};

class Server : public QTcpServer
{
    Q_OBJECT

    QTcpServer * qserver;
    database * db;

    QMap<qintptr, QTcpSocket *> SClients;

    serverStatus status;

public:
    Server();

public slots:
    void newUser();
    void slotReadyRead();

private slots:
    void forceClose();

signals:
    void newMessage(const qintptr descriptor, const QString & message);
};

#endif // SERVER_H

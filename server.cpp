#include "server.h"


Server::Server()
{
    qserver = new QTcpServer(this);
    connect(qserver, &QTcpServer::newConnection, this, &Server::newUser);

    db = new database(this);
    connect(this, &Server::newMessage, db, &database::newMessage);
    connect(this, &Server::registrate, db, &database::registrate);
    connect(this, &Server::checkUser, db, &database::checkUser);

    connect(db, &database::authorizationSuccess, this, &Server::authorizationSuccess);
    connect(db, &database::authorizationException, this, &Server::authorizationException);

    if (!qserver->listen(QHostAddress::Any, 17888)) {
        qDebug() << QObject::tr("Unable to start the server: %1.").arg(qserver->errorString());
        status = DEAD;
    } else {
        qDebug() << QString::fromUtf8("Success!");
        status = ALIVE;
    }
}

void Server::newUser()
{
    if (status == ALIVE) {
        qDebug() << "Got new connection!";
        QTcpSocket * clientSocket = qserver->nextPendingConnection();
        qintptr idUserSock = clientSocket->socketDescriptor();

        SClients[idUserSock] = clientSocket;
        connect(SClients[idUserSock], &QTcpSocket::readyRead, this, &Server::slotReadyRead);
        connect(clientSocket, &QAbstractSocket::disconnected, this, &Server::disconnectUser);
    }
}

void Server::slotReadyRead()
{
    QTcpSocket* clientSocket = static_cast<QTcpSocket*>(sender());
    qintptr idUserSock = clientSocket->socketDescriptor();

    QString tmp = QString(clientSocket->readAll());
    qDebug() << tmp;

    if (tmp.startsWith("$#$#$") && tmp.endsWith("$#$#$")) { //registration
        emit registrate(QStringList(tmp.split("$#$#$", QString::SkipEmptyParts)));
    } else if (tmp.startsWith("#%#%#") && tmp.endsWith(("#%#%#"))) { //authorization
        emit checkUser(QStringList(tmp.split(("#%#%#"), QString::SkipEmptyParts)));
        if (authorization) {
            qDebug() << "#%#%#ok#%#%#";
            clientSocket->write("#%#%#ok#%#%#");
        } else {
            qDebug() << "#%#%#no#%#%#";
            clientSocket->write("#%#%#no#%#%#");
        }
    } else {
        emit newMessage(idUserSock, tmp);
        for (auto client : SClients) {
            if (client == clientSocket) continue;
            client->write(tmp.toUtf8());
        }
        if (tmp == "goodbye") {
            clientSocket->close();
            SClients.remove(idUserSock);
            qDebug() << "NAS POKINYLI";
        }
    }
}

void Server::forceClose()
{
    if (status == ALIVE) {
        foreach(qintptr i, SClients.keys()) {
            SClients[i]->close();
            SClients.remove(i);
        }
    }

    qserver->close();
    qDebug() << QString::fromUtf8("Server closed");
    status = DEAD;
}

void Server::disconnectUser()
{
    QTcpSocket* clientSocket = static_cast<QTcpSocket*>(sender());
    qintptr idUserSock = -1;

    for ( auto key : SClients.keys() ) {
        if ( clientSocket == SClients[key] ) {
            idUserSock = key;
        }
    }

    SClients.remove(idUserSock);
    qDebug() << "User " << idUserSock << " has disconnected from your channel.";
}

void Server::authorizationSuccess()
{
    authorization = true;
}

void Server::authorizationException()
{
    authorization = false;
}


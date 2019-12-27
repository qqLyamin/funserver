#include "server.h"


Server::Server()
{
    connect(&qserver, &my_server::newConnection, this, &Server::newUser);
    connect(this, &Server::checkPendingConnection, &qserver, &my_server::nextPendingConnectionRequest);
    connect(&qserver, &my_server::nextPendingConnection, this, &Server::treatNewUser);

    db = new database(this);
    connect(this, &Server::registrate, db, &database::registrate);
    connect(this, &Server::checkUser, db, &database::checkUser);
    connect(db, &database::reg_false, this, &Server::reg_false);
    connect(db, &database::reg_true, this, &Server::reg_true);
    connect(db, &database::authorizationSuccess, this, &Server::authorizationSuccess);
    connect(db, &database::authorizationException, this, &Server::authorizationException);

    listenThread = new QThread(nullptr);
    qserver.moveToThread(listenThread);
    qserver.listen(QHostAddress::Any, 17888);
    listenThread->start();
}

void Server::newUser()
{
    qDebug() << "User connected";
    emit checkPendingConnection();
}

void Server::treatNewUser(QTcpSocket * nextPendingConnection)
{
    client * next_client = new client(nextPendingConnection);
    clientsList.push_back(next_client);

    connect(next_client, &client::readyRead, this, &Server::slotReadyRead);
    connect(this, &Server::writeToClient, next_client, &client::write);
    connect(next_client, &client::disconnected, this, &Server::disconnectUser);

    QThread * new_thread = new QThread;
    clientsThreads.push_back(new_thread);
    next_client->moveToThread(new_thread);
    new_thread->start();
}

void Server::slotReadyRead(const QByteArray & data,  qint64 clientDescriptor)
{
    QString tmp = QString(data);
    if (tmp.startsWith("$#$#$") && tmp.endsWith("$#$#$")) { //registration
        emit registrate(QStringList(tmp.split("$#$#$", QString::SkipEmptyParts)), clientDescriptor);
    } else if (tmp.startsWith("#%#%#") && tmp.endsWith(("#%#%#"))) { //authorization
        emit checkUser(QStringList(tmp.split(("#%#%#"), QString::SkipEmptyParts)));
        if (authorization) {
            for (auto client : clientsList) {
                if (client->clientSocket->socketDescriptor() == clientDescriptor) {
                    client->clientSocket->write("#%#%#ok#%#%#");
                }
            }
        } else {
            for (auto client : clientsList) {
                if (client->clientSocket->socketDescriptor() == clientDescriptor) {
                    client->clientSocket->write("#%#%#no#%#%#");
                }
            }
        }
    } else {
        for (auto client : clientsList) {
            if (client->clientSocket->socketDescriptor() == clientDescriptor) continue;
            client->clientSocket->write(tmp.toLatin1());
        }
    }
}

void Server::forceClose()
{
    if (status == ALIVE) {
        for (auto client : clientsList) {
            client->clientSocket->close();
        }
    }

    this->qserver.qserver.close();
    status = DEAD;
}

void Server::disconnectUser()
{
    client* dc_client = static_cast<client*>(sender());
    QTcpSocket* dc_socket = dc_client->clientSocket;

    for (auto client : clientsList) {
        if (client->clientSocket == dc_socket) {
            clientsThreads.removeOne(dc_client->thread());
            dc_client->thread()->terminate();
            clientsList.removeOne(client);
            delete client;
        }
    }
    qDebug() << "User disconnected";
}

void Server::authorizationSuccess()
{
    authorization = true;
}

void Server::authorizationException()
{
    authorization = false;
}

void Server::reg_false(qint64 rg_false_clientSocket)
{
    for (auto client : clientsList) {
        if (client->clientSocket->socketDescriptor() == rg_false_clientSocket) {
            client->write("$#$#$no$#$#$", rg_false_clientSocket);
        }
    }
}

void Server::reg_true(qint64 rg_true_clientSocket)
{
    for (auto client : clientsList) {
        if (client->clientSocket->socketDescriptor() == rg_true_clientSocket) {
            client->write("$#$#$ok$#$#$", rg_true_clientSocket);
        }
    }
}


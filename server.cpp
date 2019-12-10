#include "server.h"


Server::Server()
{
    qserver = new QTcpServer(this);
    connect(qserver, &QTcpServer::newConnection, this, &Server::newUser);

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
    }
}

void Server::slotReadyRead()
{
    QTcpSocket* clientSocket = static_cast<QTcpSocket*>(sender());
    qintptr idUserSock = clientSocket->socketDescriptor();

    QTextStream os(clientSocket);
    os.setAutoDetectUnicode(true);
    os << "HTTP/1.0 200 Ok\r\n"
          "Content-Type: text/html; charset=\"utf-8\"\r\n"
          "\r\n"
          "<h1>Nothing to see here</h1>\n"
          << QDateTime::currentDateTime().toString() << "\n";
    // Полученные данные от клиента выведем в qDebug,
    // можно разобрать данные например от GET запроса и по условию выдавать необходимый ответ.
    QString tmp = QString(clientSocket->readAll());
    qDebug() << tmp +"\n\r";

    if (tmp == "goodbuy") {
        clientSocket->write("NY I POSHEL NAHUI!");
        clientSocket->close();
        SClients.remove(idUserSock);
    } else {
        clientSocket->write("NY ZDAROVA EBAT'!");
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


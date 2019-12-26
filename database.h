#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>
#include <QSqlError>

class database : public QObject
{
    Q_OBJECT

    QSqlDatabase db;

public:
    explicit database(QObject *parent = nullptr);

public slots:
    void checkUser(const QStringList &);
    void registrate(const QStringList &, qint64);
    void newMessage(const qint64 descriptor, const QString & message);

signals:
    void authorizationSuccess();
    void authorizationException();
    void reg_false(qint64);
    void reg_true(qint64);
};

#endif // DATABASE_H

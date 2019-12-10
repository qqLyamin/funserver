#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

class database : public QObject
{
    Q_OBJECT

    QSqlDatabase db;

public:
    explicit database(QObject *parent = nullptr);
};

#endif // DATABASE_H

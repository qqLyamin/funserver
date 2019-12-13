#include "database.h"
#include <QTime>

database::database(QObject *)
{
    db = QSqlDatabase::addDatabase( "QSQLITE" );
    db.setDatabaseName( "workServerDB.db" );
    db.open();

    QSqlQuery query;
    query.prepare("SELECT count(*) FROM sqlite_master WHERE type='table' AND name=:tablename");
    QStringList tables{ "msgstory" };

    bool databaseValid = true;

    for( const QString & tbl : tables )
    {
        query.bindValue( ":tablename", tbl );
        if( !query.exec() ||
            !query.first()||
            !query.value(0).toInt() )
        {
            databaseValid = false;
            break;
        }
    }

    if ( databaseValid )
        qInfo( "Database has been sucessfuly loaded." );
    else
    {
        qCritical( "Could not load database." );
        db.close();
    }
}

void database::checkUser(const QStringList & name_pw)
{
    QSqlQuery query;
    query.prepare( "SELECT * FROM users WHERE name = (:name) AND password = (:password)" );
    query.bindValue( ":name", name_pw[0] );
    query.bindValue( ":password", name_pw[1] );

    if (db.isOpen()) {
        if ( query.exec() && query.first() )
        {
            qInfo() << "authorizationSuccess:" << name_pw[0];
            emit authorizationSuccess();
        } else {
            emit authorizationException();
        }
    } else {
        emit authorizationException();
        qWarning() << "db is closed";
    }
}

void database::registrate(const QStringList & name_pw)
{
    QSqlQuery query;
    query.prepare( "INSERT INTO users (name, password) VALUES (:name, :password)" );
    query.bindValue( ":name", name_pw[0] );
    query.bindValue( ":password", name_pw[1] );

    if (db.isOpen()) {
        if (query.exec()) {
            qDebug() << "added client " << name_pw[0] << " with password " << name_pw[1] << endl;
        } else {
            qWarning() << "exec problem" << query.lastError().text();
        }
    } else {
        qWarning() << "db is closed";
    }
}

void database::newMessage(const qintptr descriptor, const QString &message)
{
    QSqlQuery query;
    query.prepare( "INSERT INTO msgstory (descriptor, message, datetime) VALUES (:descriptor, :message, :datetime)" );
    query.bindValue( ":descriptor", descriptor );
    query.bindValue( ":message", message );
    query.bindValue(":datetime", QDateTime::currentDateTime());

    qDebug() << descriptor << "     " << message;
    if (db.isOpen()) {
        if (query.exec()) {
            qDebug() << "added message to db: " << message << endl;
        } else {
            qWarning() << "exec problem" << query.lastError().text();
        }
    } else {
        qWarning() << "db is closed";
    }

}

#include "database.h"

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

void database::newMessage(const qintptr descriptor, const QString &message)
{
    QSqlQuery query;
    query.prepare( "INSERT INTO msgstory (descriptor, message) VALUES (:descriptor, :message)" );
    query.bindValue( ":descriptor", descriptor );
    query.bindValue( ":message", message );

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

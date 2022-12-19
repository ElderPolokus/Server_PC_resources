#ifndef DB_H
#define DB_H

#include <QSqlDatabase>
#include <QString>

class DB
{
public:
    DB();

public:
    void connectToDB();
    void insertLogTimeDB(QString IP, QString status_log);
};

#endif // DB_H

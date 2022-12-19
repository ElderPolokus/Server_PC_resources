#include "db.h"
#include <QSqlDatabase>
#include <QMessageBox>
#include <QtSql>

DB::DB() {

}

void DB::connectToDB() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("DB_server_connections");
    db.setUserName("postgres");
    db.setPassword("root");
    if(!db.open()) {
        QMessageBox::warning(0, "Ошибка при открытии БД", db.lastError().text());
    } else {
        QMessageBox::information(0, "Подключение к БД", "Соединение с БД установлено!");
    }
}

void DB::insertLogTimeDB(QString IP, QString status_log) {
    QDateTime dTime = QDateTime::currentDateTime();
    QSqlQuery query;
    //В зависимости от переданного статуса обновляем таблицу
    if(status_log == "Connected") {
        //Проверка на существование IP в базе и добавление статуса
        query.exec("SELECT EXISTS (SELECT * FROM Users WHERE IPv4Address_user = '"+IP+"') ");
        query.next();
        if(query.value(0) == 0) {
            query.exec("INSERT INTO Users (IPv4Address_user, LogOnTime) VALUES ('"+IP+"', 'Connected')");
        } else {
            query.exec("UPDATE Users SET LogOnTime = 'Connected' WHERE IPv4Address_user = '"+IP+"' ");
        }
    } else if(IP == "shutdown server") {
        query.exec("UPDATE Users SET LogOnTime = '"+status_log+"' WHERE LogOnTime = 'Connected' ");
    } else {
        query.exec("UPDATE Users SET LogOnTime = '"+status_log+"' WHERE IPv4Address_user = '"+IP+"' ");
    }

}

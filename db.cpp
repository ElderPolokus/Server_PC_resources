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
    if(status_log == "Connected") {
        query.exec("INSERT INTO Users (IPv4Address_user, LogOnTime) VALUES ('"+IP+"', '"+dTime.toString()+"')");
    } else if(IP == "shutdown server") {
        query.exec("UPDATE Users SET LogOffTime = '"+dTime.toString()+"' WHERE LogOffTime IS NULL ");
    } else {
        query.exec("UPDATE Users SET LogOffTime = '"+dTime.toString()+"' WHERE IPv4Address_user = '"+IP+"' AND LogOffTime IS NULL ");
    }
}

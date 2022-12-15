#ifndef CONNECTTODB_H
#define CONNECTTODB_H

#include <QSqlDatabase>
#include <QMessageBox>
#include <QtSql>

bool createConnectionDB() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("DB_server_connections");
    db.setUserName("postgres");
    db.setPassword("root");
    if(!db.open()) {
        QMessageBox::warning(0, "Ошибка при открытии БД", db.lastError().text());
        return false;
    } else {
        QMessageBox::information(0, "Подключение к БД", "Соединение с БД установлено!");
        return true;
    }
}

#endif // CONNECTTODB_H

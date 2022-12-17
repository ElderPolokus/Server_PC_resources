#include "server.h"
#include "ui_server.h"
#include <QMessageBox>
#include <QTime>
#include <QSqlQueryModel>
#include "connectToDB.h"
#include "resourceTableModel.h"

server::server(int nPort, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::server)
    , m_nNextBlockSize(0)
{
    ui->setupUi(this);
    //resourcesModel

    //DataBase
    createConnectionDB();
    refreshStatusInfo();
    query = new QSqlQuery(db);
    //Запуск сервера
    m_pTcpServer = new QTcpServer(this);
    if (!m_pTcpServer->listen(QHostAddress::Any, nPort)) {
        QMessageBox::critical(0, "Server Error", "Не удалось запустить сервер: " + m_pTcpServer->errorString());
        m_pTcpServer->close();
        return;
    }
    //Если ошибки нет, соединяем слот с сигналом newConnection()
    connect(m_pTcpServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
}

server::~server()
{
    delete ui;
}

void server::slotNewConnection() {
    //Сокет для связи с клиентом
    QTcpSocket* pClientSocket = m_pTcpServer->nextPendingConnection();
    //Вытаскиваем IP-адрес клиента
    QString IP = ((QHostAddress)pClientSocket->peerAddress().toIPv4Address()).toString();
    statusInfo(IP);
    query->exec("UPDATE Users SET LogOnTime = 'Connected' WHERE IPv4Address_user = '"+IP+"' ");
    refreshStatusInfo();
    //При отключении клиента сервер удалит сокет
    connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));
    connect(pClientSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    connect(pClientSocket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));
}

void server::slotDisconnected() {
    QString IP;
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    IP = ((QHostAddress)pClientSocket->peerAddress().toIPv4Address()).toString();
    query->exec("UPDATE Users SET LogOnTime = 'Disconnected' WHERE IPv4Address_user = '"+IP+"' ");
    refreshStatusInfo();
}

void server::slotReadClient() {
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    //В класс для работы с информацией помещаем данные из сокета
    QDataStream in(pClientSocket);
    //Указываем версию для исключения ошибок
    in.setVersion(QDataStream::Qt_6_3);
    for(;;) {
        if (!m_nNextBlockSize) {
            if (pClientSocket->bytesAvailable() < (int)sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if(pClientSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }
        // Переменные для получения ифнормации по ресурсам
        int cpu_value, ram_value, disk_value;
        QString disk_name;

        in >> cpu_value >> ram_value >> disk_name >> disk_value;
        //Передаем данные в модель таблицы
        resourcesInfo(cpu_value, ram_value, disk_name, disk_value);
        m_nNextBlockSize = 0;
    }
}

void server::refreshStatusInfo() {
    model = new QSqlTableModel(this, db);
    model->setTable("users");
    model->select();
    ui->infoView->setModel(model);
    ui->infoView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void server::statusInfo(QString IP) {
    query->exec("SELECT EXISTS (SELECT * FROM Users WHERE IPv4Address_user = '"+IP+"') ");
    query->next();
    if(query->value(0) == 0) {
        query->exec("INSERT INTO Users (IPv4Address_user) VALUES ('"+IP+"')");
    }
    refreshStatusInfo();
}

void server::resourcesInfo(int cpu, int ram, QString disk_name, int disk_value) {
    int nRows = 0;
    query->exec("SELECT COUNT(IPv4Address_user) FROM Users WHERE LogOnTime = 'Connected' ");
    if(query->next()) {
        nRows = query->value(0).toInt();
        query->clear();
        query->exec("SELECT IPv4Address_user FROM Users WHERE LogOnTime = 'Connected' ");
        resourceData* resModel = new resourceData(nRows, this);
        for(int i = 1; i<nRows+1; i++) {
            if(query->next()) {
                resModel->setResource(i, 1, query->value(0).toString());
                resModel->setResource(i, 2, QString::number(cpu));
                resModel->setResource(i, 3, QString::number(ram));
                resModel->setResource(i, 4, disk_name);
                resModel->setResource(i, 5, QString::number(disk_value));
            }
        }
        ui->resourcesView->setModel(resModel);
    }
}

void server::closeEvent(QCloseEvent* e) {
    query->exec("UPDATE Users SET LogOnTime = 'Disconnected'");
}

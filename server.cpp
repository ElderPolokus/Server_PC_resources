#include "server.h"
#include "ui_server.h"
#include <QMessageBox>
#include <QTime>
#include <QSqlQueryModel>
#include "db.h"
#include "resourceTableModel.h"

server::server(int nPort, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::server)
    , m_nNextBlockSize(0)
{
    ui->setupUi(this);
    //resourcesModel
    model = new resourceTableModel;
    ui->resourcesView->setModel(model);
    //DataBase
    DB db;
    db.connectToDB();
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
    DB db;
    db.insertLogTimeDB(IP, "Connected");

    //При отключении клиента сервер удалит сокет
    connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));
    connect(pClientSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
    connect(pClientSocket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));
}

void server::slotDisconnected() {
    QString IP;
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    IP = ((QHostAddress)pClientSocket->peerAddress().toIPv4Address()).toString();
    if(model->findIP(IP) != -1) {
        model->removeRowRes(IP);
    }
    //Обновляем данные в БД
    DB db;
    db.insertLogTimeDB(IP, "Disconnected");
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
        QString disk_name, IP_cl;

        in >> IP_cl >>cpu_value >> ram_value >> disk_name >> disk_value;
        //Передаем данные в модель таблицы
        resourcesInfo(IP_cl, cpu_value, ram_value, disk_name, disk_value);
        m_nNextBlockSize = 0;
    }
}

void server::resourcesInfo(QString IP_cl, int cpu, int ram, QString disk_name, int disk_value) {
    if(model->findIP(IP_cl) != -1) {
        int row = model->findIP(IP_cl);
        model->setResource(row, 2, QString::number(cpu));
        model->setResource(row, 3, QString::number(ram));
        model->setResource(row, 4, disk_name);
        model->setResource(row, 5, QString::number(disk_value));
    } else {
        model->appendClient(IP_cl, cpu, ram, disk_name, disk_value);
    }
}

void server::closeEvent(QCloseEvent* e) {
    Q_UNUSED(e);
    DB db;
    db.insertLogTimeDB("shutdown server", "shutdown server");
}

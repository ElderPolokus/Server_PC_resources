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
        QTime time;
        QString str;
        in >> time >> str;
        QString strMessage = str;
        ui->m_ptxt->append(strMessage);
        //Передаем данные в модель таблицы
        resourcesInfo(strMessage);
        m_nNextBlockSize = 0;
    }
}

void server::sendToClient(QTcpSocket* pSocket, const QString& str) {
    QByteArray arrBlock;
    //Данные на вывод
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_3);
    out << quint16(0) << QTime::currentTime() << str;
    out.device()->seek(0);
    out <<quint16(arrBlock.size() - sizeof(quint16));
    pSocket->write(arrBlock);
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

void server::resourcesInfo(QString strMes) {
    //Разбиваем полученные данные на строки
    QStringList listData = strMes.split(" ", Qt::SkipEmptyParts);
    //Передаем данные в таблицу
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
                resModel->setResource(i, 2, listData.at(0));
                resModel->setResource(i, 3, listData.at(1));
                resModel->setResource(i, 4, listData.at(2) + listData.at(3) + " " + listData.at(4));
                resModel->setResource(i, 5, listData.at(5));
            }
        }
        ui->resourcesView->setModel(resModel);
    }
//    resModel->setResource(1, 1, "Ko");
//    resModel->setResource(1, 2, "Op");
//    resModel->insertRow(resModel->rowCount(QModelIndex()));
}

void server::closeEvent(QCloseEvent* e) {
    query->exec("UPDATE Users SET LogOnTime = 'Disconnected'");
}

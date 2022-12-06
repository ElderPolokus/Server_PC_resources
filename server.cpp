#include "server.h"
#include "ui_server.h"
#include <QMessageBox>
#include <QTime>
#include <QSqlQueryModel>

server::server(int nPort, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::server)
    , m_nNextBlockSize(0)
{
    ui->setupUi(this);
    //DataBase
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/Qt/qt_projects/Server/DB_server_connect.db");
    if(!db.open()) {
        qDebug() << db.lastError().text();
    }
    refreshInfo();
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
    QHostAddress ip4Address(pClientSocket->peerAddress().toIPv4Address());
    ui->infoTxt->append(ip4Address.toString());
    statusInfo(ip4Address.toString());
    //При отключении клиента сервер удалит сокет
    connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));
    connect(pClientSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected(ip4Address.toString())));
    connect(pClientSocket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));
//    sendToClient(pClientSocket, "Server Response: Connected!");
}

void server::slotDisconnected(QString IP) {
    QSqlQuery query;
    query.exec("UPDATE Users SET LogOnTime = 'Disconnect' WHERE IPv4Address_user = '"+IP+"' ");
    refreshInfo();
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
        QString strMessage = time.toString() + " " + "Client has sent - " + str;
        ui->m_ptxt->append(strMessage);

        m_nNextBlockSize = 0;

        sendToClient(pClientSocket, "Server Response: Received \"" + str +"\"");
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

void server::refreshInfo() {
    model = new QSqlTableModel(this, db);
    model->setTable("Users");
    model->select();
    ui->infoView->setModel(model);
    ui->infoView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void server::statusInfo(QString IP) {
    QSqlQuery query;
    query.exec("SELECT EXISTS (SELECT * FROM Users WHERE IPv4Address_user = '"+IP+"') ");
    query.next();
    if(query.value(0) == 0) {
        query.exec("INSERT INTO Users (IPv4Address_user) VALUES ('"+IP+"')");
    }
    refreshInfo();
}

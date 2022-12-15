#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtSql>

QT_BEGIN_NAMESPACE
namespace Ui { class server; }
QT_END_NAMESPACE

class server : public QMainWindow
{
    Q_OBJECT

public:
    server(int nPort, QWidget *parent = nullptr);
    ~server();

private:
    Ui::server *ui;
    QTcpServer* m_pTcpServer;
    quint16 m_nNextBlockSize;
    void sendToClient(QTcpSocket* pSocket, const QString& str);
    QSqlDatabase db;
    QSqlQuery* query;
    QSqlTableModel* model;

protected:
    void closeEvent(QCloseEvent* e);

public slots:
    virtual void slotNewConnection();
    void slotReadClient();
    void slotDisconnected();
    void statusInfo(QString IP);
    void refreshStatusInfo();
    void resourcesInfo(QString strMes);
};

#endif // SERVER_H

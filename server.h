#ifndef SERVER_H
#define SERVER_H

#include "resourceTableModel.h"
#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtSql>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class server; }
QT_END_NAMESPACE

class server : public QMainWindow
{
    Q_OBJECT

public:
    server(QWidget *parent = nullptr);
    ~server();

private:
    Ui::server *ui;
    QTcpServer* m_pTcpServer;
    quint16 m_nNextBlockSize;
    void sendToClient(QTcpSocket* pSocket, const QString& str);
    resourceTableModel* model;

protected:
    void closeEvent(QCloseEvent* e);

public slots:
    virtual void slotNewConnection();
    void slotReadClient();
    void slotDisconnected();
    void resourcesInfo(QString IP_cl, int cpu, int ram, QString disk_name, int disk_value);
};

#endif // SERVER_H

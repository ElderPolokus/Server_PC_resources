#include "server.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    server server;
    server.show();
    return a.exec();
}

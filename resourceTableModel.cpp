#include "resourceTableModel.h"
#include <QLocale>

resourceTableModel::resourceTableModel(QObject *parent)
    : QAbstractTableModel{parent}
{

}

int resourceTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_resources.count();
}

int resourceTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return LAST;
}

QVariant resourceTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Vertical) {
        return QString("Клиент %1").arg(section+1);
    }

    switch (section) {
    case IPv4Address:
        return QString("IP клиента");
    case CPU_val:
        return QString("ЦП");
    case RAM_val:
        return QString("ОЗУ");
    case DISK_name:
        return QString("Имя диска");
    case DISK_val:
        return QString("Диск");
    default:
        return QString("Столбец %1").arg(section+1);
    }
}

QVariant resourceTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || m_resources.count() <= index.row() || (role != Qt::DisplayRole && role != Qt::EditRole)) {
        return QVariant();
    }
    return m_resources[ index.row() ][ Column(index.column()) ];
}

bool resourceTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole && m_resources.count() > index.row()) {
        m_resources[ index.row() ][ Column(index.column()) ] = value;
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

void resourceTableModel::appendClient(QString& IPAdrs, int& cpu_val, int& ram_val, QString disk_name, int disk_val) {
    clientData clData;
    clData[ IPv4Address ] = IPAdrs;
    clData[ CPU_val ] = cpu_val;
    clData[ RAM_val ] = ram_val;
    clData[ DISK_name ] = disk_name;
    clData[ DISK_val ] = disk_val;

    int row = m_resources.count();
    beginInsertRows(QModelIndex(), row, row);
    m_resources.append(clData);
    endInsertRows();
}

int resourceTableModel::findIP(QString IPAdrs) {
    int i = 1;
    Resources::Iterator iter;
    for(iter = m_resources.begin(); iter != m_resources.end(); ++iter) {
        if((iter->value(IPv4Address)) == IPAdrs) {
            return i;
        } else {
            i++;
        }
    }
    return -1;
}

void resourceTableModel::setResource(int clientNum, int resourceType, QString course) {
    setData(index(clientNum-1, resourceType-1), course, Qt::EditRole);
}

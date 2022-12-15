#include "resourceTableModel.h"
#include <QLocale>

resourceTableModel::resourceTableModel(int nRows, int nColumns, QObject *parent)
    : QAbstractTableModel{parent}
    , m_nRows(nRows)
    , m_nColumns(nColumns)
{

}

int resourceTableModel::rowCount(const QModelIndex &parent) const {
    return m_nRows;
}

int resourceTableModel::columnCount(const QModelIndex &parent) const {
    return m_nColumns;
}

QVariant resourceTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    return (role == Qt::DisplayRole || role == Qt::EditRole) ? m_hash.value(index, QVariant()) : QVariant();
}

bool resourceTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
        m_hash[index] = value;
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

resourceData::resourceData(int nRows, QObject *parent) : resourceTableModel(nRows, 5, parent) {

}

void resourceData::setResource(int clientNum, int resourceType, QString course) {
    setData(index(clientNum-1, resourceType-1), course, Qt::EditRole);
}

QVariant resourceData::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if(orientation == Qt::Horizontal) {
        switch (section+1) {
        case 1:
            return QString("IP клиента");
        case 2:
            return QString("ЦП");
        case 3:
            return QString("ОЗУ");
        case 4:
            return QString("Имя диска");
        case 5:
            return QString("Диск");
        default:
            return QString("Столбец %1").arg(section+1);
        }
    } else {
        return QString("Клиент %1").arg(section+1);
    }
}

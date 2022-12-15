#ifndef RESOURCETABLEMODEL_H
#define RESOURCETABLEMODEL_H

#include <QAbstractTableModel>

class resourceTableModel : public QAbstractTableModel
{
public:
    explicit resourceTableModel(int nRows, int nColumns, QObject *parent = nullptr);

private:
    int m_nRows;
    int m_nColumns;

protected:
    QHash<QModelIndex, QVariant> m_hash;

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
};

class resourceData : public resourceTableModel
{
    Q_OBJECT

public:
    resourceData(int nRows, QObject *parent = nullptr);
    void setResource(int clientNum,int resourceType, QString course);

    // QAbstractItemModel interface
public:
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

#endif // RESOURCETABLEMODEL_H

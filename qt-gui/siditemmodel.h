#ifndef SIDITEMMODEL_H
#define SIDITEMMODEL_H

#include <QAbstractItemModel>

class SidItem;
class SidItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SidItemModel(QObject *parent = 0);
    ~SidItemModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    SidItem *itemFromModelIndex(const QModelIndex &index) const;
    void directoryData(const QModelIndex& modelIndex, const QStringList& directories, const QStringList& sidfiles);
signals:
    void fetchItem(SidItem *item);

public slots:

private:
    void setupModelData(const QStringList &lines, SidItem *parent);
    SidItem *rootItem;
};

#endif // SIDITEMMODEL_H
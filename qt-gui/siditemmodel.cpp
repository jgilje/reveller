#include "siditemmodel.h"
#include "siditem.h"

#include <QDebug>
#include <QFileIconProvider>

SidItemModel::SidItemModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    rootItem = new SidItem("ROOT");
}

SidItemModel::~SidItemModel() {
}

QModelIndex SidItemModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    SidItem *parentItem = itemFromModelIndex(parent);
    SidItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    else
        return QModelIndex();
}

QModelIndex SidItemModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    SidItem *childItem = static_cast<SidItem*>(index.internalPointer());
    SidItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int SidItemModel::rowCount(const QModelIndex &parent) const {
    SidItem *parentItem;
    if (parent.column() > 0){
        return 0;
    }

    parentItem = itemFromModelIndex(parent);
    return parentItem->childCount();
}

int SidItemModel::columnCount(const QModelIndex &parent) const {
    SidItem *parentItem = itemFromModelIndex(parent);
    switch (parentItem->type()) {
    case SidItem::DIRECTORY:
        return 1;
    case SidItem::SIDFILE:
        return 0;
    }

    // return parentItem->childCount();

    /*
    if (parent.isValid())
        return static_cast<SidItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
    */
}

QVariant SidItemModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    SidItem *item = static_cast<SidItem*>(index.internalPointer());
    QFileIconProvider iconprovider;

    switch (role) {
    case Qt::DisplayRole:
        return item->name();
    case Qt::DecorationRole:
        switch (item->type()) {
        case SidItem::DIRECTORY:
            return iconprovider.icon(QFileIconProvider::Folder);
        case SidItem::SIDFILE:
            return iconprovider.icon(QFileIconProvider::File);
        }

        break;
    default:
        // qDebug() << role;
        return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags SidItemModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return 0;

    SidItem* item = itemFromModelIndex(index);

    switch (item->type()) {
    case SidItem::DIRECTORY:
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    case SidItem::SIDFILE:
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
    }

    return 0;
}

QVariant SidItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
    /*
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);
    */

    return QVariant(QString("HeaderData %1, %2").arg(section).arg(orientation));
}

void SidItemModel::directoryData(const QModelIndex& modelIndex, const QStringList &directories, const QStringList &sidfiles) {
    beginInsertRows(modelIndex, 0, directories.size() + sidfiles.size());
    SidItem *parentItem = itemFromModelIndex(modelIndex);

    foreach (const QString& dir, directories) {
        parentItem->appendDirectory(dir);
    }
    foreach (const QString& sidfile, sidfiles) {
        parentItem->appendSidFile(sidfile);
    }
    parentItem->setLoaded();

    endInsertRows();
}

SidItem* SidItemModel::itemFromModelIndex(const QModelIndex &modelIndex) const {
    SidItem *item;

    if (!modelIndex.isValid())
        item = rootItem;
    else
        item = static_cast<SidItem*>(modelIndex.internalPointer());

    return item;
}

bool SidItemModel::canFetchMore(const QModelIndex &parent) const {
    SidItem *item = itemFromModelIndex(parent);
    return !item->isLoading() && !item->hasLoaded();
}

void SidItemModel::fetchMore(const QModelIndex &parent) {
    SidItem *item = itemFromModelIndex(parent);
    item->setLoading();
    emit fetchItem(item);
}

bool SidItemModel::hasChildren(const QModelIndex &parent) const {
    SidItem *item = itemFromModelIndex(parent);
    if (item->type() == SidItem::DIRECTORY) {
        return true;
    }

    return false;
}

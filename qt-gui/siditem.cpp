#include "siditem.h"

#include <QDebug>

SidItem::SidItem(const QString& name, Type type, SidItem *parent) :
    _loaded(false),
    _loading(false)
{
    _parent = parent;
    _type = type;
    _name = name;
}

SidItem::~SidItem() {
    qDeleteAll(childItems);
}

void SidItem::appendChild(SidItem *item) {
    childItems.append(item);
}

SidItem *SidItem::child(int row) {
    return childItems.value(row);
}

SidItem *SidItem::child(const QString& name) {
    foreach (SidItem* item, childItems) {
        if (item->name() == name) {
            return item;
        }
    }

    return NULL;
}

int SidItem::childCount() const {
    return childItems.count();
}

int SidItem::row() const {
    if (_parent)
        return _parent->childItems.indexOf(const_cast<SidItem*>(this));

    return 0;
}

int SidItem::columnCount() const {
    return 1;
}

SidItem *SidItem::parent() {
    return _parent;
}

void SidItem::appendDirectory(const QString &dir) {
    SidItem* item = new SidItem(dir, DIRECTORY, this);
    item->_loaded = false;
    appendChild(item);
}

void SidItem::appendSidFile(const QString &sidfile) {
    SidItem* item = new SidItem(sidfile, SIDFILE, this);
    item->_loaded = true;
    appendChild(item);
}

const QString& SidItem::name() const {
    return _name;
}

QString SidItem::path() const {
    if (_parent == NULL) {
        return "";
    }

    QString p = _parent->path();
    if (p.isEmpty()) {
        return _name;
    } else {
        return _parent->path() + "/" + _name;
    }
}

SidItem::Type SidItem::type() const {
    return _type;
}

bool SidItem::hasLoaded() const {
    return _loaded;
}

void SidItem::setLoaded() {
    _loaded = true;
}

bool SidItem::isLoading() const {
    return _loading;
}

void SidItem::setLoading() {
    _loading = true;
}

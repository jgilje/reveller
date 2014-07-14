#include "siditem.h"

SidItem::SidItem(const QString& name, SidItem *parent) :
    _loaded(false),
    _loading(false)
{
    _parent = parent;
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
    SidItem* item = new SidItem(dir, this);
    item->_type = DIRECTORY;
    item->_loaded = false;
    appendChild(item);
}

void SidItem::appendSidFile(const QString &sidfile) {
    SidItem* item = new SidItem(sidfile, this);
    item->_type = SIDFILE;
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

    return _parent->path() + "/" + _name;
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

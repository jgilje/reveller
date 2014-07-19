#ifndef SIDITEM_H
#define SIDITEM_H

#include <QVariant>

class SidItem {
public:
    enum Type {
        DIRECTORY,
        SIDFILE
    };

    SidItem(const QString& name, Type type, SidItem *parent = 0);
    ~SidItem();

    void appendDirectory(const QString& dir);
    void appendSidFile(const QString& sidfile);

    const QString& name() const;
    QString path() const;
    Type type() const;
    bool hasLoaded() const;
    void setLoaded();
    bool isLoading() const;
    void setLoading();

    SidItem *child(int row);
    SidItem *child(const QString& name);
    int childCount() const;
    int columnCount() const;
    int row() const;
    SidItem *parent();
private:
    void appendChild(SidItem *child);

    QList<SidItem*> childItems;
    SidItem *_parent;
    QString _name;

    Type _type;
    bool _loaded;
    bool _loading;
};

#endif // SIDITEM_H

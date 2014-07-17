#ifndef SIDINFO_H
#define SIDINFO_H

#include <QWidget>

namespace Ui {
class SidInfo;
}

class SidInfo : public QWidget
{
    Q_OBJECT

public:
    explicit SidInfo(QWidget *parent = 0);
    ~SidInfo();

    void name(const QString& name);
    void author(const QString& author);
    void released(const QString& released);
private:
    Ui::SidInfo *ui;
};

#endif // SIDINFO_H

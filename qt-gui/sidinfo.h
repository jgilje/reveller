#ifndef SIDINFO_H
#define SIDINFO_H

#include <QWidget>

#include "sidheader.h"

namespace Ui {
class SidInfo;
}

class SidInfo : public QWidget
{
    Q_OBJECT

public:
    explicit SidInfo(QWidget *parent = 0);
    ~SidInfo();

    void setHeader(const SidHeader& header);
private:
    Ui::SidInfo *ui;
    SidHeader _header;
};

#endif // SIDINFO_H

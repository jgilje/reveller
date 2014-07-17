#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectDialog;
}

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(QWidget *parent = 0);
    ~ConnectDialog();

    QString url();
    void url(const QString& url);
private:
    Ui::ConnectDialog *ui;
};

#endif // CONNECTDIALOG_H

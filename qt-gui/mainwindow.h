#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWebSockets/QWebSocket>

#include "siditemmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QWebSocket _webSocket;
    SidItemModel *model;

    void handleLs(const QJsonArray& directories, const QJsonArray& sidfiles);
    void handleLoad(const QString& data);
    void handleState(const QJsonObject& data);
    void handleStateChange(const QString& data);
private slots:
    void onConnected();
    void onDisconnected();
    void onFetchItem(SidItem *item);
    void onTextMessageReceived(QString msg);
    void onUpdatePreview(const QModelIndex &index);
};

#endif // MAINWINDOW_H
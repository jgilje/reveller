#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWebSockets/QWebSocket>

#include <QItemSelection>

#include "siditemmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(bool &ok, QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QWebSocket *_webSocket;
    SidItemModel *model;

    QString runConnectionDialog();
    void wsConnect(const QString& address);
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
    void onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void onWebSocketStateChange(QAbstractSocket::SocketState state);
};

#endif // MAINWINDOW_H

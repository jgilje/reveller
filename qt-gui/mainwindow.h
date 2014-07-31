#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWebSockets/QWebSocket>

#include <QItemSelection>

#include "siditemmodel.h"
#include "sidheader.h"

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
    SidItemModel *_model;
    SidHeader header;
    QStringList selectPath;
    uint currentSong;
    QString currentFile;

    QString runConnectionDialog();
    void fetchPath(const QString& path);
    void resolveSelectPath();
    void updateNavbar(const QString& file);
    void wsConnect(const QString& address);
    void songRequest();

    void handleLs(const QString& path, const QJsonArray& directories, const QJsonArray& sidfiles);
    void handleLoad(const QString& data);
    void handleState(const QJsonObject& data);
private slots:
    void onConnected();
    void onNavigation(const QString & link);
    void onDisconnected();
    void onFetchItem(SidItem *item);
    void onTextMessageReceived(QString msg);
    void onUpdatePreview(const QModelIndex &index);
    void onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void onWebSocketStateChange(QAbstractSocket::SocketState state);

    void on_actionDisconnect_triggered();
    void on_pushButtonPrev_clicked();
    void on_pushButtonNext_clicked();
};

#endif // MAINWINDOW_H

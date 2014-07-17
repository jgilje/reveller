#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "siditem.h"
#include "connectdialog.h"
#include "sidinfo.h"

#include <QSettings>
#include <QMessageBox>

MainWindow::MainWindow(bool &ok, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _webSocket(NULL),
    _model(NULL)
{
    ui->setupUi(this);

    SidInfo *info = new SidInfo;
    ui->columnView->setPreviewWidget(info);

    QSettings settings;
    QString url = settings.value("url").toString();
    if (url.isEmpty()) {
        url = runConnectionDialog();
        if (url.isEmpty()) {
            ok = false;
            return;
        }
        wsConnect(url);
    } else {
        wsConnect(url);
    }

    ok = true;
}

MainWindow::~MainWindow() {
    delete ui;
}

QString MainWindow::runConnectionDialog() {
    QSettings settings;
    ConnectDialog d;
    d.url(settings.value("url").toString());
    int r = d.exec();
    if (r == 0) {
        return "";
    }

    return d.url();
}

void MainWindow::wsConnect(const QString &address) {
    if (_webSocket != NULL) {
        delete _webSocket;
    }

    QUrl url(address);
    QUrl wsUrl(url);
    wsUrl.setScheme("ws");
    wsUrl.setPath("/ws");

    _webSocket = new QWebSocket(address);
    connect(_webSocket, &QWebSocket::connected, this, &MainWindow::onConnected);
    connect(_webSocket, &QWebSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(_webSocket, &QWebSocket::stateChanged, this, &MainWindow::onWebSocketStateChange);

    _webSocket->open(wsUrl);
}

void MainWindow::onWebSocketStateChange(QAbstractSocket::SocketState state) {
    qDebug() << state;
}

void MainWindow::onConnected() {
    QSettings settings;
    settings.setValue("url", _webSocket->origin());

    connect(_webSocket, &QWebSocket::textMessageReceived, this, &MainWindow::onTextMessageReceived);

    _model = new SidItemModel();
    connect(_model, &SidItemModel::fetchItem, this, &MainWindow::onFetchItem);
    ui->columnView->setModel(_model);

    connect(ui->columnView, &QColumnView::updatePreviewWidget, this, &MainWindow::onUpdatePreview);
    connect(ui->columnView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onSelectionChanged);

    QJsonObject json;
    json["action"] = "state";
    _webSocket->sendTextMessage(QJsonDocument(json).toJson());
}

void MainWindow::onDisconnected() {
    qDebug() << "WebSocket disconnected";
    _webSocket->close();

    QString url = runConnectionDialog();
    if (url.isEmpty()) {
        qApp->exit(1);
    }

    wsConnect(url);
}

void MainWindow::onTextMessageReceived(QString message) {
    QJsonDocument document = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject root = document.object();
    QString type = root["type"].toString();
    QString data = root["data"].toString();

    QJsonDocument dataDocument = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject dataObj = dataDocument.object();

    if (type == QStringLiteral("state")) {
        handleState(dataObj);
    } else if (type == QStringLiteral("ls")) {
        handleLs(dataObj.value("directories").toArray(), dataObj.value("sidfiles").toArray());
    } else if (type == QStringLiteral("load")) {
        handleLoad(data);
    } else if (type == QStringLiteral("stateChange")) {
        handleStateChange(data);
    } else if (type == QStringLiteral("sidHeader")) {
        header = SidHeader::parse(dataObj);
        SidInfo *info = qobject_cast<SidInfo*>(ui->columnView->previewWidget());
        info->name(header.name);
        info->author(header.author);
        info->released(header.released);
    } else if (type == QStringLiteral("crash")) {
        QMessageBox::warning(this, "Crash!", QString("The SIDPlayer crashed with the following error \"%1\"").arg(data.trimmed()));
    } else {
        qDebug() << message;
    }
}

void MainWindow::handleState(const QJsonObject &data) {
    // TODO - song is not exposed yet
    ui->labelCurrentSong->setText(data["file"].toString());
    ui->labelCurrentState->setText(data["state"].toString() == "play" ? "playing" : "stopped");
}

void MainWindow::handleLs(const QJsonArray &jsonDirectories, const QJsonArray &jsonSidfiles) {
    QStringList directories;
    QStringList sidfiles;

    foreach (const QJsonValue& value, jsonDirectories) {
        directories.append(value.toString());
    }
    foreach (const QJsonValue& value, jsonSidfiles) {
        sidfiles.append(value.toString());
    }

    _model->directoryData(ui->columnView->selectionModel()->currentIndex(), directories, sidfiles);
}

void MainWindow::handleLoad(const QString &data) {
    ui->labelCurrentSong->setText(data);
}

void MainWindow::handleStateChange(const QString &data) {
    ui->labelCurrentState->setText(data == "play" ? "playing" : "stopped");
}

void MainWindow::onFetchItem(SidItem *item) {
    if (item->hasLoaded()) {
        return;
    }

    QJsonObject json;
    json["action"] = "ls";
    json["argument"] = item->path();
    _webSocket->sendTextMessage(QJsonDocument(json).toJson());
}

void MainWindow::onUpdatePreview(const QModelIndex &index) {
    SidItem *item = _model->itemFromModelIndex(index);

    QJsonObject json;
    json["action"] = "load";
    json["argument"] = item->path();
    _webSocket->sendTextMessage(QJsonDocument(json).toJson());

    json["action"] = "song";
    json["argument"] = "0";
    _webSocket->sendTextMessage(QJsonDocument(json).toJson());
}

void MainWindow::onSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    /* Prevents annoying select-all => select actual behaviour when clicking a sid file */
    if (ui->columnView->selectionModel()->selectedIndexes().size() > 0) {
        ui->columnView->selectionModel()->select(ui->columnView->selectionModel()->selectedIndexes().last(), QItemSelectionModel::SelectCurrent);
    }
}

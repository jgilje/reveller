#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "siditem.h"

#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _webSocket(QString("http://192.168.0.149:8080"))
{
    ui->setupUi(this);

    connect(&_webSocket, &QWebSocket::connected, this, &MainWindow::onConnected);
    connect(&_webSocket, &QWebSocket::disconnected, this, &MainWindow::onDisconnected);

    _webSocket.open(QUrl("ws://192.168.0.149:8080/ws"));

    model = new SidItemModel;
    ui->columnView->setModel(model);

    QLabel *label = new QLabel;
    ui->columnView->setPreviewWidget(label);

    /*
    connect(ui->columnView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::onNavigation);
    */
    connect(model, &SidItemModel::fetchItem, this, &MainWindow::onFetchItem);
    connect(ui->columnView, &QColumnView::updatePreviewWidget, this, &MainWindow::onUpdatePreview);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnected() {
    connect(&_webSocket, &QWebSocket::textMessageReceived, this, &MainWindow::onTextMessageReceived);

    QJsonObject json;
    json["action"] = "state";
    _webSocket.sendTextMessage(QJsonDocument(json).toJson());

    json["action"] = "ls";
    _webSocket.sendTextMessage(QJsonDocument(json).toJson());
}

void MainWindow::onDisconnected() {
    qDebug() << "WebSocket disconnected";
    _webSocket.close();
}

void MainWindow::onTextMessageReceived(QString message) {
    QJsonDocument document = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject root = document.object();
    QString type = root["type"].toString();
    QString data = root["data"].toString();

    QJsonDocument dataDocument = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject dataObj = dataDocument.object();

    if (type == QString("state")) {
        handleState(dataObj);
    } else if (type == QString("ls")) {
        handleLs(dataObj.value("directories").toArray(), dataObj.value("sidfiles").toArray());
    } else if (type == QString("load")) {
        handleLoad(data);
    } else if (type == QString("stateChange")) {
        handleStateChange(data);
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

    model->directoryData(ui->columnView->selectionModel()->currentIndex(), directories, sidfiles);
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
    _webSocket.sendTextMessage(QJsonDocument(json).toJson());
}

void MainWindow::onUpdatePreview(const QModelIndex &index) {
    SidItem *item = model->itemFromModelIndex(index);
    ui->columnView->previewWidget()->setProperty("text", item->name());

    QJsonObject json;
    json["action"] = "load";
    json["argument"] = item->path();
    _webSocket.sendTextMessage(QJsonDocument(json).toJson());

    json["action"] = "song";
    json["argument"] = "0";
    _webSocket.sendTextMessage(QJsonDocument(json).toJson());
}

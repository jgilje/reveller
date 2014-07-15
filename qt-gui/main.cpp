#include "mainwindow.h"
#include <QApplication>

#include <QtWebSockets/QtWebSockets>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Carbonium Development");
    QCoreApplication::setOrganizationDomain("jgilje.net");
    QCoreApplication::setApplicationName("SID Player");

    bool ok;
    MainWindow w(ok);
    if (! ok) {
        return 1;
    }

    w.show();
    return a.exec();
}

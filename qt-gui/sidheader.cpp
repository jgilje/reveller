#include "sidheader.h"

#include <QJsonArray>
#include <QDebug>

SidHeader::SidHeader() :
    basicRom(0),
    c64Compatible(0),
    computePlayer(0),
    internalPlayer(0),
    playSidSamples(0),

    systemHz(0),
    songs(0),
    startSong(0),
    version(0),

    pageLength(0),
    startPage(0),

    dataOffset(0),
    initAddress(0),
    loadAddress(0),
    playAddress(0),

    type(Type_Unknown),
    model(Model_Unknown),
    hz(Hz_Unknown),
    interrupts()
{
}

SidHeader SidHeader::parse(const QJsonObject &jsonObj) {
    SidHeader sh;

    QString type = jsonObj["Type"].toString();
    if (type == QStringLiteral("PSID")) {
        sh.type = Type_PSID;
    } else if (type == QStringLiteral("RSID")) {
        sh.type = Type_RSID;
    } else {
        sh.type = Type_Unknown;
        return sh;
    }

    sh.systemHz = jsonObj["Hz"].toInt();
    sh.songs = jsonObj["Songs"].toInt();
    sh.startSong = jsonObj["StartSong"].toInt();
    sh.version = jsonObj["Version"].toInt();

    sh.pageLength = jsonObj["PageLength"].toInt();
    sh.startPage = jsonObj["StartPage"].toInt();

    sh.dataOffset = jsonObj["DataOffset"].toInt();
    sh.initAddress = jsonObj["InitAddress"].toInt();
    sh.loadAddress = jsonObj["LoadAddress"].toInt();
    sh.playAddress = jsonObj["PlayAddress"].toInt();

    QJsonObject flags = jsonObj["Flags"].toObject();
    sh.basicRom = flags["BASIC_ROM"].toBool();
    sh.c64Compatible = flags["C64Compatible"].toBool();
    sh.computePlayer = flags["ComputePlayer"].toBool();
    sh.internalPlayer = flags["InternalPlayer"].toBool();
    sh.playSidSamples = flags["PlaySIDSamples"].toBool();
    QString sidModel = flags["SIDModel"].toString();
    if (sidModel == QStringLiteral("6581")) {
        sh.model = Model_6581;
    } else if (sidModel == QStringLiteral("8580")) {
        sh.model = Model_8580;
    } else if (sidModel == QStringLiteral("Both")) {
        sh.model = Model_Both;
    } else if (sidModel == QStringLiteral("Unknown")) {
        sh.model = Model_Unknown;
    } else {
        sh.model = Model_Unknown;
    }

    QString sidHz = flags["Speed"].toString();
    if (sidHz == QStringLiteral("PAL")) {
        sh.hz = Hz_PAL;
    } else if (sidHz == QStringLiteral("NTSC")) {
        sh.hz = Hz_NTSC;
    } else if (sidHz == QStringLiteral("Both")) {
        sh.hz = Hz_Both;
    } else if (sidHz == QStringLiteral("Unknown")) {
        sh.hz = Hz_Unknown;
    } else {
        sh.hz = Hz_Unknown;
    }

    QJsonArray speedArray = jsonObj["Speed"].toArray();
    if (speedArray.size() >= 32) {
        for (int i = 0; i < 32; i++) {
            const QJsonValue& value = speedArray[i];
            sh.interrupts[i] = (value.toString() == QStringLiteral("VBI")) ? Interrupt_VBI : Interrupt_CIA;
        }

    }

    sh.author = jsonObj["Author"].toString();
    sh.name = jsonObj["Name"].toString();
    sh.released = jsonObj["Released"].toString();

    return sh;
}

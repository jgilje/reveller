#ifndef SIDHEADER_H
#define SIDHEADER_H

#include <QJsonObject>

class SidHeader {
public:
    explicit SidHeader();

    enum SIDType {
        Type_Unknown,
        Type_PSID,
        Type_RSID
    };

    enum SIDModel {
        Model_Unknown,
        Model_6581,
        Model_8580,
        Model_Both
    };

    enum SIDHz {
        Hz_Unknown,
        Hz_PAL,
        Hz_NTSC,
        Hz_Both
    };

    enum SIDInterrupt {
        Interrupt_VBI,
        Interrupt_CIA
    };

    static SidHeader parse(const QJsonObject& jsonObj);

    bool basicRom;
    bool c64Compatible;
    bool computePlayer;
    bool internalPlayer;
    bool playSidSamples;

    uint systemHz;
    uint songs;
    uint startSong;
    uint version;

    u_int8_t pageLength;
    u_int8_t startPage;

    ushort dataOffset;
    ushort initAddress;
    ushort loadAddress;
    ushort playAddress;

    SIDType type;
    SIDModel model;
    SIDHz hz;
    SIDInterrupt interrupts[32];

    QString author;
    QString name;
    QString released;
};

#endif // SIDHEADER_H

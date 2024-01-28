#ifndef NFCMANAGER_H
#define NFCMANAGER_H

#include <QObject>
#include <QMessageBox>

#include <nfc/nfc.h>
#include <unistd.h>
#include <sstream>
#include <cstdint>
#include <iomanip>



class NFCManager : public QObject
{
    Q_OBJECT
public:
    explicit NFCManager(QObject *parent = nullptr);

    bool isCardAttached();
    QString getFormatHex(const uint8_t *pbtData, const size_t &szBytes);
    QString getCardUID();



private:
    nfc_context *m_context;
    nfc_device *m_device;

    const nfc_modulation nmMifare = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
    };

};

#endif // NFCMANAGER_H

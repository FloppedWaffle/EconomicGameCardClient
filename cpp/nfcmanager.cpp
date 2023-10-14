#include "nfcmanager.h"


NFCManager::NFCManager(QObject *parent)
    : QObject{parent}
{

}



bool NFCManager::isCardAttached()
{
    size_t devicesCount;
    nfc_connstring devices[3];
    nfc_device *device;
    nfc_target target;

    nfc_init(&m_context);
    if (m_context == nullptr)
    {
        QMessageBox::critical(nullptr,
        "Ошибка",
        "Ошибка инициализации libnfc");
        return false;
    }

    devicesCount = nfc_list_devices(m_context, devices, 3);

    if (!bool(devicesCount))
    {
        nfc_exit(m_context);
        return false;
    }

    for (size_t  i = 0; i < devicesCount; i++)
    {
        device = nfc_open(m_context, devices[i]);
        int deviceInit = nfc_initiator_init(device);

        if (!device || deviceInit < 0)
        {
            continue;
        }

        if (nfc_initiator_select_passive_target(device, nmMifare, nullptr, 0, &target) > 0)
        {
            m_device = device;
            return true;
        }
    }

    return false;
}



QString NFCManager::getFormatHex(const uint8_t *pbtData, const size_t &szBytes)
{
    std::ostringstream oss;

    for (size_t szPos = 0; szPos < szBytes; szPos++)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(pbtData[szPos]) << " ";
    }

    QString str = QString::fromStdString(oss.str());
    return str.trimmed();
}



QString NFCManager::getCardUID()
{
    if (!isCardAttached())
    {
        return "";
    }

    QString uid = "";
    nfc_target target;

    if (nfc_initiator_select_passive_target(m_device, nmMifare, nullptr, 0, &target) > 0)
    {
        uid = getFormatHex(target.nti.nai.abtUid, target.nti.nai.szUidLen);
    }

    return uid;
}

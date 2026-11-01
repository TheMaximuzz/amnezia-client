#ifndef LOCALINTERFACECONTROLLER_H
#define LOCALINTERFACECONTROLLER_H

#include <QObject>
#include <QString>

#include "core/controllers/serversController.h"
#include "core/controllers/selfhosted/installController.h"
#include "core/repositories/secureAppSettingsRepository.h"
#include "core/utils/errorCodes.h"

struct LocalInterfaceSettings
{
    bool hasTunnelConfig = false;
    QString address;
    QString privateKey;
    QString mtu;
    QString dns;
    int listenPort = 0;
    QString preUpScript;
    QString postUpScript;
    QString preDownScript;
    QString postDownScript;
    int scriptTimeout = 10;
    bool allowLocalTraffic = false;
};

class LocalInterfaceController : public QObject
{
    Q_OBJECT

public:
    LocalInterfaceController(ServersController *serversController, InstallController *installController,
                             SecureAppSettingsRepository *appSettingsRepository, QObject *parent = nullptr);

    LocalInterfaceSettings load() const;
    amnezia::ErrorCode save(const LocalInterfaceSettings &settings);

    static QString setInterfaceValue(const QString &nativeConfig, const QString &key, const QString &value);

private:
    ServersController *m_serversController;
    InstallController *m_installController;
    SecureAppSettingsRepository *m_appSettingsRepository;
};

#endif // LOCALINTERFACECONTROLLER_H

#include "localInterfaceUiController.h"

#include "core/configurators/wireguardConfigurator.h"
#include "core/utils/errorStrings.h"

LocalInterfaceUiController::LocalInterfaceUiController(LocalInterfaceController *localInterfaceController, QObject *parent)
    : QObject(parent), m_localInterfaceController(localInterfaceController)
{
}

QVariantMap LocalInterfaceUiController::load() const
{
    const LocalInterfaceSettings settings = m_localInterfaceController->load();
    return {
        { "hasTunnelConfig", settings.hasTunnelConfig },
        { "address", settings.address },
        { "privateKey", settings.privateKey },
        { "mtu", settings.mtu },
        { "dns", settings.dns },
        { "listenPort", settings.listenPort > 0 ? QString::number(settings.listenPort) : QString() },
        { "preUpScript", settings.preUpScript },
        { "postUpScript", settings.postUpScript },
        { "preDownScript", settings.preDownScript },
        { "postDownScript", settings.postDownScript },
        { "scriptTimeout", QString::number(settings.scriptTimeout) },
        { "allowLocalTraffic", settings.allowLocalTraffic },
    };
}

void LocalInterfaceUiController::save(const QVariantMap &values)
{
    LocalInterfaceSettings settings;
    settings.address = values.value("address").toString();
    settings.privateKey = values.value("privateKey").toString();
    settings.mtu = values.value("mtu").toString();
    settings.dns = values.value("dns").toString();
    settings.listenPort = values.value("listenPort").toString().toInt();
    settings.preUpScript = values.value("preUpScript").toString();
    settings.postUpScript = values.value("postUpScript").toString();
    settings.preDownScript = values.value("preDownScript").toString();
    settings.postDownScript = values.value("postDownScript").toString();
    settings.scriptTimeout = qMax(1, values.value("scriptTimeout").toString().toInt());
    settings.allowLocalTraffic = values.value("allowLocalTraffic").toBool();

    const amnezia::ErrorCode errorCode = m_localInterfaceController->save(settings);
    if (errorCode != amnezia::ErrorCode::NoError) {
        emit errorOccurred(errorString(errorCode));
        return;
    }
    emit finished(tr("Settings updated successfully"));
}

QString LocalInterfaceUiController::generatePrivateKey() const
{
    return WireguardConfigurator::genClientKeys().clientPrivKey;
}

QString LocalInterfaceUiController::publicKey(const QString &privateKey) const
{
    return WireguardConfigurator::publicKeyFromPrivate(privateKey.trimmed());
}

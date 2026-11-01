#include "localInterfaceController.h"

#include <QRegularExpression>

#include "core/configurators/wireguardConfigurator.h"

using namespace amnezia;

namespace
{
    template<typename Function>
    bool withTunnelClientConfig(ContainerConfig &containerConfig, Function &&function)
    {
        if (auto *awg = containerConfig.getAwgProtocolConfig(); awg && awg->clientConfig.has_value()) {
            function(*awg->clientConfig);
            return true;
        }
        if (auto *wireGuard = containerConfig.getWireGuardProtocolConfig(); wireGuard && wireGuard->clientConfig.has_value()) {
            function(*wireGuard->clientConfig);
            return true;
        }
        return false;
    }

    const QStringList scriptStages { "preUp", "postUp", "preDown", "postDown" };
}

LocalInterfaceController::LocalInterfaceController(ServersController *serversController, InstallController *installController,
                                                   SecureAppSettingsRepository *appSettingsRepository, QObject *parent)
    : QObject(parent),
      m_serversController(serversController),
      m_installController(installController),
      m_appSettingsRepository(appSettingsRepository)
{
}

LocalInterfaceSettings LocalInterfaceController::load() const
{
    LocalInterfaceSettings settings;

    const QString serverId = m_serversController->getDefaultServerId();
    ContainerConfig containerConfig = m_serversController->getContainerConfig(serverId, m_serversController->getDefaultContainer(serverId));
    settings.hasTunnelConfig = withTunnelClientConfig(containerConfig, [&settings](const auto &clientConfig) {
        settings.address = clientConfig.clientIp;
        settings.privateKey = clientConfig.clientPrivateKey;
        settings.mtu = clientConfig.mtu;
    });

    QStringList dnsServers { m_appSettingsRepository->primaryDns(), m_appSettingsRepository->secondaryDns() };
    dnsServers.removeAll(QString());
    settings.dns = dnsServers.join(", ");

    settings.listenPort = m_appSettingsRepository->listenPort();
    settings.preUpScript = m_appSettingsRepository->connectionScript(scriptStages.at(0));
    settings.postUpScript = m_appSettingsRepository->connectionScript(scriptStages.at(1));
    settings.preDownScript = m_appSettingsRepository->connectionScript(scriptStages.at(2));
    settings.postDownScript = m_appSettingsRepository->connectionScript(scriptStages.at(3));
    settings.scriptTimeout = m_appSettingsRepository->connectionScriptTimeout();
    settings.allowLocalTraffic = m_appSettingsRepository->isLocalTrafficAllowed();
    return settings;
}

ErrorCode LocalInterfaceController::save(const LocalInterfaceSettings &settings)
{
    const QStringList dnsServers = settings.dns.split(QRegularExpression("[,;\\s]+"), Qt::SkipEmptyParts);
    if (!dnsServers.isEmpty()) {
        m_appSettingsRepository->setPrimaryDns(dnsServers.value(0));
        m_appSettingsRepository->setSecondaryDns(dnsServers.value(1));
    }

    m_appSettingsRepository->setListenPort(settings.listenPort);
    m_appSettingsRepository->setConnectionScript(scriptStages.at(0), settings.preUpScript);
    m_appSettingsRepository->setConnectionScript(scriptStages.at(1), settings.postUpScript);
    m_appSettingsRepository->setConnectionScript(scriptStages.at(2), settings.preDownScript);
    m_appSettingsRepository->setConnectionScript(scriptStages.at(3), settings.postDownScript);
    m_appSettingsRepository->setConnectionScriptTimeout(settings.scriptTimeout);
    m_appSettingsRepository->setLocalTrafficAllowed(settings.allowLocalTraffic);

    const QString serverId = m_serversController->getDefaultServerId();
    const DockerContainer container = m_serversController->getDefaultContainer(serverId);
    ContainerConfig containerConfig = m_serversController->getContainerConfig(serverId, container);

    const bool hasTunnelConfig = withTunnelClientConfig(containerConfig, [&settings](auto &clientConfig) {
        clientConfig.clientIp = settings.address.trimmed();
        clientConfig.mtu = settings.mtu.trimmed();
        if (clientConfig.clientPrivateKey != settings.privateKey.trimmed()) {
            clientConfig.clientPrivateKey = settings.privateKey.trimmed();
            clientConfig.clientPublicKey = WireguardConfigurator::publicKeyFromPrivate(clientConfig.clientPrivateKey);
        }
        clientConfig.nativeConfig = setInterfaceValue(clientConfig.nativeConfig, "Address", clientConfig.clientIp);
        clientConfig.nativeConfig = setInterfaceValue(clientConfig.nativeConfig, "PrivateKey", clientConfig.clientPrivateKey);
        clientConfig.nativeConfig = setInterfaceValue(clientConfig.nativeConfig, "MTU", clientConfig.mtu);
    });

    if (!hasTunnelConfig) {
        return ErrorCode::NoError;
    }
    return m_installController->updateClientConfig(serverId, container, containerConfig);
}

QString LocalInterfaceController::setInterfaceValue(const QString &nativeConfig, const QString &key, const QString &value)
{
    if (nativeConfig.isEmpty()) {
        return nativeConfig;
    }

    QStringList lines = nativeConfig.split('\n');
    const QRegularExpression keyLine(QString("^\\s*%1\\s*=").arg(QRegularExpression::escape(key)), QRegularExpression::CaseInsensitiveOption);

    int interfaceStart = -1;
    int interfaceEnd = lines.size();
    for (int i = 0; i < lines.size(); ++i) {
        const QString section = lines.at(i).trimmed();
        if (!section.startsWith('[')) {
            continue;
        }
        if (interfaceStart >= 0) {
            interfaceEnd = i;
            break;
        }
        if (section.compare("[Interface]", Qt::CaseInsensitive) == 0) {
            interfaceStart = i;
        }
    }
    if (interfaceStart < 0) {
        return nativeConfig;
    }

    for (int i = interfaceStart + 1; i < interfaceEnd; ++i) {
        if (keyLine.match(lines.at(i)).hasMatch()) {
            if (value.isEmpty()) {
                lines.removeAt(i);
            } else {
                lines[i] = QString("%1 = %2").arg(key, value);
            }
            return lines.join('\n');
        }
    }

    if (!value.isEmpty()) {
        lines.insert(interfaceStart + 1, QString("%1 = %2").arg(key, value));
    }
    return lines.join('\n');
}

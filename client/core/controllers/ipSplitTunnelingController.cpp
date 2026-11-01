#include "ipSplitTunnelingController.h"
#include "core/utils/networkUtilities.h"
#include <QJsonObject>
#include <QDebug>
#include <QHostAddress>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace
{
    const QList<QUrl> ruSubnetsUrls {
        QUrl("https://raw.githubusercontent.com/ipverse/rir-ip/master/country/ru/ipv4-aggregated.txt"),
        QUrl("https://www.ipdeny.com/ipblocks/data/aggregated/ru-aggregated.zone"),
    };
    constexpr int ruSubnetsMaxAgeDays = 7;
    constexpr int ruSubnetsCheckIntervalMs = 60 * 60 * 1000;
    constexpr int ruSubnetsRequestTimeoutMs = 30000;
    constexpr int ruSubnetsMinPrefixLength = 8;
    constexpr int ruSubnetsMinCount = 100;
}

IpSplitTunnelingController::IpSplitTunnelingController(SecureAppSettingsRepository* appSettingsRepository, QObject* parent)
    : QObject(parent),
      m_appSettingsRepository(appSettingsRepository)
{
    m_currentRouteMode = m_appSettingsRepository->routeMode();
    if (m_currentRouteMode == RouteMode::VpnAllSites) { // for old split tunneling configs
        m_appSettingsRepository->setRouteMode(RouteMode::VpnOnlyForwardSites);
        m_currentRouteMode = RouteMode::VpnOnlyForwardSites;
    }
    fillSites();

    m_ruSubnetsCheckTimer.setInterval(ruSubnetsCheckIntervalMs);
    connect(&m_ruSubnetsCheckTimer, &QTimer::timeout, this, &IpSplitTunnelingController::updateRuSubnetsIfStale);
    m_ruSubnetsCheckTimer.start();
    QTimer::singleShot(0, this, &IpSplitTunnelingController::updateRuSubnetsIfStale);
}

bool IpSplitTunnelingController::isRuSubnetsBypassEnabled() const
{
    return m_appSettingsRepository->isRuSubnetsBypassEnabled();
}

void IpSplitTunnelingController::setRuSubnetsBypassEnabled(bool enabled)
{
    m_appSettingsRepository->setRuSubnetsBypassEnabled(enabled);
    if (enabled) {
        updateRuSubnetsIfStale();
    }
}

int IpSplitTunnelingController::ruSubnetsCount() const
{
    return m_appSettingsRepository->ruSubnets().size();
}

QDateTime IpSplitTunnelingController::ruSubnetsUpdatedAt() const
{
    return m_appSettingsRepository->ruSubnetsUpdatedAt();
}

bool IpSplitTunnelingController::isRuSubnetsUpdating() const
{
    return m_ruSubnetsUpdating;
}

void IpSplitTunnelingController::updateRuSubnetsIfStale()
{
    if (!m_appSettingsRepository->isRuSubnetsBypassEnabled()) {
        return;
    }

    const QDateTime updatedAt = m_appSettingsRepository->ruSubnetsUpdatedAt();
    const bool isStale = !updatedAt.isValid() || updatedAt.addDays(ruSubnetsMaxAgeDays) <= QDateTime::currentDateTimeUtc();
    if (isStale || m_appSettingsRepository->ruSubnets().isEmpty()) {
        updateRuSubnets();
    }
}

void IpSplitTunnelingController::updateRuSubnets()
{
    if (m_ruSubnetsUpdating) {
        return;
    }
    m_ruSubnetsUpdating = true;
    emit ruSubnetsUpdateStarted();
    downloadRuSubnets(0, {});
}

void IpSplitTunnelingController::downloadRuSubnets(int sourceIndex, const QStringList &errors)
{
    if (sourceIndex >= ruSubnetsUrls.size()) {
        m_ruSubnetsUpdating = false;
        emit ruSubnetsUpdateFailed(tr("Failed to download the list of Russian addresses: %1").arg(errors.join("; ")));
        return;
    }

    QNetworkRequest request(ruSubnetsUrls.at(sourceIndex));
    request.setTransferTimeout(ruSubnetsRequestTimeoutMs);
    QNetworkReply *reply = m_networkManager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, sourceIndex, errors]() {
        reply->deleteLater();
        const QString host = reply->url().host();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[SplitTunneling] RU subnets download failed:" << host << reply->errorString();
            downloadRuSubnets(sourceIndex + 1, errors + QStringList { QString("%1: %2").arg(host, reply->errorString()) });
            return;
        }

        const QStringList subnets = parseRuSubnets(reply->readAll());
        if (subnets.size() < ruSubnetsMinCount) {
            qWarning() << "[SplitTunneling] RU subnets list rejected:" << host << "entries:" << subnets.size();
            downloadRuSubnets(sourceIndex + 1, errors + QStringList { QString("%1: %2").arg(host, tr("invalid list")) });
            return;
        }

        m_ruSubnetsUpdating = false;
        m_appSettingsRepository->setRuSubnets(subnets, QDateTime::currentDateTimeUtc());
        qDebug() << "[SplitTunneling] RU subnets updated from" << host << "entries:" << subnets.size();
        emit ruSubnetsUpdated();
    });
}

QStringList IpSplitTunnelingController::parseRuSubnets(const QByteArray &data)
{
    QStringList subnets;
    const QList<QByteArray> lines = data.split('\n');
    for (const QByteArray &rawLine : lines) {
        const QString line = QString::fromLatin1(rawLine).section('#', 0, 0).trimmed();
        if (line.isEmpty()) {
            continue;
        }

        const QPair<QHostAddress, int> subnet = QHostAddress::parseSubnet(line.contains('/') ? line : line + "/32");
        if (subnet.first.protocol() != QAbstractSocket::IPv4Protocol || subnet.second < ruSubnetsMinPrefixLength) {
            continue;
        }

        const quint32 mask = subnet.second == 0 ? 0 : ~quint32(0) << (32 - subnet.second);
        const QString network = QHostAddress(subnet.first.toIPv4Address() & mask).toString();
        subnets.append(QString("%1/%2").arg(network).arg(subnet.second));
    }
    subnets.removeDuplicates();
    return subnets;
}

bool IpSplitTunnelingController::addSiteToMode(RouteMode mode, const QString &hostname)
{
    const QString normalizedHostname = normalizeHostname(hostname);
    if (!validateHostname(normalizedHostname)) {
        return false;
    }

    if (!m_appSettingsRepository->addVpnSite(mode, normalizedHostname)) {
        return false;
    }

    if (!NetworkUtilities::ipAddressWithSubnetRegExp().exactMatch(normalizedHostname)) {
        QHostInfo::lookupHost(normalizedHostname, this, [this, mode, normalizedHostname](const QHostInfo &hostInfo) {
            QStringList ips;
            for (const QHostAddress &address : hostInfo.addresses()) {
                if (address.protocol() == QAbstractSocket::IPv4Protocol) {
                    ips.append(address.toString());
                }
            }
            if (!ips.isEmpty()) {
                m_appSettingsRepository->addVpnSite(mode, normalizedHostname, ips);
                if (mode == m_currentRouteMode) {
                    fillSites();
                }
            }
        });
    }

    if (mode == m_currentRouteMode) {
        fillSites();
    }
    applyForwardAndExceptLists();
    return true;
}

void IpSplitTunnelingController::removeSiteFromMode(RouteMode mode, const QString &hostname)
{
    m_appSettingsRepository->removeVpnSite(mode, hostname);
    if (mode == m_currentRouteMode) {
        fillSites();
    }
    applyForwardAndExceptLists();
}

QVector<QPair<QString, QStringList>> IpSplitTunnelingController::sitesForMode(RouteMode mode) const
{
    QVector<QPair<QString, QStringList>> sites;
    const QVariantMap sitesMap = m_appSettingsRepository->vpnSites(mode);
    for (auto it = sitesMap.constBegin(); it != sitesMap.constEnd(); ++it) {
        sites.append(qMakePair(it.key(), SecureAppSettingsRepository::siteIpList(it.value())));
    }
    return sites;
}

void IpSplitTunnelingController::applyForwardAndExceptLists()
{
    const bool hasForwardSites = !m_appSettingsRepository->vpnSites(RouteMode::VpnOnlyForwardSites).isEmpty();
    const bool hasExceptSites = !m_appSettingsRepository->vpnSites(RouteMode::VpnAllExceptSites).isEmpty()
            || m_appSettingsRepository->isRuSubnetsBypassEnabled();

    const RouteMode effectiveMode = hasForwardSites ? RouteMode::VpnOnlyForwardSites : RouteMode::VpnAllExceptSites;
    if (effectiveMode != m_currentRouteMode) {
        setRouteMode(effectiveMode);
    }
    toggleSplitTunneling(hasForwardSites || hasExceptSites);
}

bool IpSplitTunnelingController::addSiteInternal(const QString &hostname, const QStringList &ips)
{
    QVariantMap existing = m_appSettingsRepository->vpnSites(m_currentRouteMode);
    if (existing.contains(hostname) && ips.isEmpty()) {
        return false;
    }

    for (int i = 0; i < m_sites.size(); i++) {
        if (m_sites[i].first == hostname) {
            bool changed = false;
            for (const QString &ip : ips) {
                if (!ip.isEmpty() && !m_sites[i].second.contains(ip)) {
                    m_sites[i].second.append(ip);
                    changed = true;
                }
            }
            if (!changed) {
                return false;
            }
            m_appSettingsRepository->addVpnSite(m_currentRouteMode, hostname, ips);
            return true;
        }
    }
    m_sites.append(qMakePair(hostname, ips));
    m_appSettingsRepository->addVpnSite(m_currentRouteMode, hostname, ips);
    return true;
}

void IpSplitTunnelingController::addSites(const QMap<QString, QStringList> &sites, bool replaceExisting)
{
    if (replaceExisting) {
        m_sites.clear();
    }
    for (auto it = sites.constBegin(); it != sites.constEnd(); ++it) {
        const QString &hostname = it.key();
        const QStringList &ips = it.value();
        bool found = false;
        for (int i = 0; i < m_sites.size(); i++) {
            if (m_sites[i].first == hostname) {
                for (const QString &ip : ips) {
                    if (!ip.isEmpty() && !m_sites[i].second.contains(ip)) {
                        m_sites[i].second.append(ip);
                    }
                }
                found = true;
                break;
            }
        }
        if (!found) {
            m_sites.append(qMakePair(hostname, ips));
        }
    }
    if (replaceExisting) {
        m_appSettingsRepository->removeAllVpnSites(m_currentRouteMode);
    }
    m_appSettingsRepository->addVpnSites(m_currentRouteMode, sites);
}

bool IpSplitTunnelingController::addSite(const QString &hostname)
{
    QString normalizedHostname = normalizeHostname(hostname);
    
    if (!validateHostname(normalizedHostname)) {
        return false;
    }
    
    if (NetworkUtilities::ipAddressWithSubnetRegExp().exactMatch(normalizedHostname)) {
        processSite(normalizedHostname, {});
        return true;
    }
    
    if (addSiteInternal(normalizedHostname, {})) {
        QHostInfo::lookupHost(normalizedHostname, this, SLOT(onHostResolved(QHostInfo)));
        return true;
    }
    
    return false;
}

bool IpSplitTunnelingController::removeSite(const QString &hostname)
{
    for (int i = 0; i < m_sites.size(); i++) {
        if (m_sites[i].first == hostname) {
            m_sites.removeAt(i);
            m_appSettingsRepository->removeVpnSite(m_currentRouteMode, hostname);
            return true;
        }
    }
    return false;
}

void IpSplitTunnelingController::removeSites()
{
    m_sites.clear();
    m_appSettingsRepository->removeAllVpnSites(m_currentRouteMode);
}

void IpSplitTunnelingController::setRouteMode(RouteMode routeMode)
{
    m_currentRouteMode = routeMode;
    fillSites();
    m_appSettingsRepository->setRouteMode(routeMode);
}

void IpSplitTunnelingController::toggleSplitTunneling(bool enabled)
{
    m_appSettingsRepository->setSitesSplitTunnelingEnabled(enabled);
}

RouteMode IpSplitTunnelingController::getRouteMode() const
{
    return m_currentRouteMode;
}

bool IpSplitTunnelingController::isSplitTunnelingEnabled() const
{
    return m_appSettingsRepository->isSitesSplitTunnelingEnabled();
}

QVector<QPair<QString, QStringList>> IpSplitTunnelingController::getCurrentSites() const
{
    return m_sites;
}

void IpSplitTunnelingController::fillSites()
{
    QVariantMap sitesMap = m_appSettingsRepository->vpnSites(m_currentRouteMode);
    m_sites.clear();
    for (auto it = sitesMap.begin(); it != sitesMap.end(); ++it) {
        m_sites.append(qMakePair(it.key(), SecureAppSettingsRepository::siteIpList(it.value())));
    }
}

QString IpSplitTunnelingController::normalizeHostname(const QString &hostname)
{
    QString normalized = hostname;
    normalized.replace("https://", "");
    normalized.replace("http://", "");
    normalized.replace("ftp://", "");

    if (NetworkUtilities::ipAddressWithSubnetRegExp().exactMatch(normalized)) {
        return normalized;
    }

    const QStringList parts = normalized.split("/", Qt::SkipEmptyParts);
    return parts.isEmpty() ? QString() : parts.first();
}

bool IpSplitTunnelingController::validateHostname(const QString &hostname)
{
    if (hostname.isEmpty()) {
        return false;
    }
    if (!hostname.contains(".") && !NetworkUtilities::ipAddressWithSubnetRegExp().exactMatch(hostname)) {
        return false;
    }
    return true;
}


void IpSplitTunnelingController::onHostResolved(const QHostInfo &hostInfo)
{
    const QList<QHostAddress> &addresses = hostInfo.addresses();
    QString hostname = hostInfo.hostName();

    QStringList allIpv4;
    for (const QHostAddress &addr : addresses) {
        if (addr.protocol() == QAbstractSocket::NetworkLayerProtocol::IPv4Protocol) {
            allIpv4.append(addr.toString());
        }
    }
    allIpv4.removeDuplicates();
    qDebug() << "[SplitTunneling] Host resolved:" << hostname
             << "-> adding all IPv4 addresses to list:" << allIpv4;

    if (!allIpv4.isEmpty()) {
        processSiteAfterResolve(hostname, allIpv4);
    }
}

void IpSplitTunnelingController::processSiteAfterResolve(const QString &hostname, const QStringList &ips)
{
    for (int i = 0; i < m_sites.size(); i++) {
        if (m_sites[i].first == hostname) {
            for (const QString &ip : ips) {
                if (!ip.isEmpty() && !m_sites[i].second.contains(ip)) {
                    m_sites[i].second.append(ip);
                }
            }
            break;
        }
    }
    m_appSettingsRepository->addVpnSite(m_currentRouteMode, hostname, ips);
}

void IpSplitTunnelingController::processSite(const QString &hostname, const QStringList &ips)
{
    addSiteInternal(hostname, ips);
}

bool IpSplitTunnelingController::importSitesFromJson(const QByteArray& jsonData, bool replaceExisting, QString &errorMessage)
{
    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        errorMessage = tr("Failed to parse JSON data: %1").arg(parseError.errorString());
        return false;
    }
    
    if (!jsonDocument.isArray()) {
        errorMessage = tr("The JSON data is not an array");
        return false;
    }
    
    QJsonArray jsonArray = jsonDocument.array();
    QMap<QString, QStringList> sites;
    
    for (auto jsonValue : jsonArray) {
        QJsonObject jsonObject = jsonValue.toObject();
        QString hostname = jsonObject.value("hostname").toString("");

        QStringList ips;
        if (jsonObject.value("ips").isArray()) {
            const QJsonArray ipsArray = jsonObject.value("ips").toArray();
            for (const auto &ipValue : ipsArray) {
                ips.append(ipValue.toString());
            }
        }
        const QString singleIp = jsonObject.value("ip").toString("");
        if (!singleIp.isEmpty()) {
            ips.append(singleIp);
        }
        ips.removeAll(QString());
        ips.removeDuplicates();
        
        QString normalizedHostname = normalizeHostname(hostname);
        
        if (!validateHostname(normalizedHostname)) {
            qDebug() << normalizedHostname << " not look like ip adress or domain name";
            continue;
        }
        
        sites.insert(normalizedHostname, ips);
    }
    
    addSites(sites, replaceExisting);
    
    return true;
}

QByteArray IpSplitTunnelingController::exportSitesToJson() const
{
    QVector<QPair<QString, QStringList>> sites = getCurrentSites();
    QJsonArray jsonArray;
    
    for (const auto &site : sites) {
        QJsonObject jsonObject;
        jsonObject["hostname"] = site.first;

        QJsonArray ipsArray;
        for (const QString &ip : site.second) {
            ipsArray.append(ip);
        }
        jsonObject["ips"] = ipsArray;
        // Keep the legacy "ip" field (first address) for backward compatibility.
        jsonObject["ip"] = site.second.isEmpty() ? QString() : site.second.first();

        jsonArray.append(jsonObject);
    }
    
    QJsonDocument jsonDocument(jsonArray);
    return jsonDocument.toJson();
}


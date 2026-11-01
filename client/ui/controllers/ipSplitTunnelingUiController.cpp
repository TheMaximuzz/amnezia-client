#include "ipSplitTunnelingUiController.h"

#include <QDebug>
#include <QLocale>

#include "systemController.h"
#include "core/utils/errorCodes.h"
#include "core/utils/routeModes.h"
#include "core/utils/commonStructs.h"

IpSplitTunnelingUiController::IpSplitTunnelingUiController(IpSplitTunnelingController* ipSplitTunnelingController,
                                                          IpSplitTunnelingModel* ipSplitTunnelingModel, QObject *parent)
    : QObject(parent),
      m_ipSplitTunnelingController(ipSplitTunnelingController),
      m_ipSplitTunnelingModel(ipSplitTunnelingModel)
{
    m_ipSplitTunnelingModel->updateModel(m_ipSplitTunnelingController->getCurrentSites());

    connect(m_ipSplitTunnelingController, &IpSplitTunnelingController::ruSubnetsUpdateStarted, this,
            &IpSplitTunnelingUiController::ruSubnetsStateChanged);
    connect(m_ipSplitTunnelingController, &IpSplitTunnelingController::ruSubnetsUpdated, this,
            &IpSplitTunnelingUiController::ruSubnetsStateChanged);
    connect(m_ipSplitTunnelingController, &IpSplitTunnelingController::ruSubnetsUpdateFailed, this,
            [this](const QString &errorMessage) {
                emit ruSubnetsStateChanged();
                emit errorOccurred(errorMessage);
            });
}

void IpSplitTunnelingUiController::addSite(QString hostname)
{
    if (m_ipSplitTunnelingController->addSite(hostname)) {
        emit finished(tr("New site added: %1").arg(hostname));
    }
}

void IpSplitTunnelingUiController::removeSite(int index)
{
    auto modelIndex = m_ipSplitTunnelingModel->index(index);
    auto hostname = m_ipSplitTunnelingModel->data(modelIndex, IpSplitTunnelingModel::Roles::UrlRole).toString();
    if (m_ipSplitTunnelingController->removeSite(hostname)) {
        emit finished(tr("Site removed: %1").arg(hostname));
    }
}

void IpSplitTunnelingUiController::removeSites()
{
    m_ipSplitTunnelingController->removeSites();
    emit finished(tr("Site list cleared!"));
}

void IpSplitTunnelingUiController::importSites(const QString &fileName, bool replaceExisting)
{
    QByteArray jsonData;
    if (!SystemController::readFile(fileName, jsonData)) {
        emit errorOccurred(tr("Can't open file: %1").arg(fileName));
        return;
    }

    QString errorMessage;
    if (m_ipSplitTunnelingController->importSitesFromJson(jsonData, replaceExisting, errorMessage)) {
        emit finished(tr("Import completed"));
    } else {
        emit errorOccurred(errorMessage);
    }
}

void IpSplitTunnelingUiController::exportSites(const QString &fileName)
{
    QByteArray jsonData = m_ipSplitTunnelingController->exportSitesToJson();
    if (!SystemController::saveFile(fileName, jsonData)) {
        qInfo() << "IpSplitTunnelingUiController::exportSites: save or share was cancelled or failed";
        return;
    }
    emit finished(tr("Export completed"));
}

void IpSplitTunnelingUiController::toggleSplitTunneling(bool enabled)
{
    m_ipSplitTunnelingController->toggleSplitTunneling(enabled);
    emit isSplitTunnelingEnabledChanged();
}

void IpSplitTunnelingUiController::setRouteMode(int routeMode)
{
    m_ipSplitTunnelingController->setRouteMode(static_cast<amnezia::RouteMode>(routeMode));
    emit routeModeChanged();
}

void IpSplitTunnelingUiController::toggleRuSubnetsBypass(bool enabled)
{
    m_ipSplitTunnelingController->setRuSubnetsBypassEnabled(enabled);
    emit ruSubnetsStateChanged();
}

void IpSplitTunnelingUiController::updateRuSubnets()
{
    m_ipSplitTunnelingController->updateRuSubnets();
}

bool IpSplitTunnelingUiController::isRuSubnetsBypassEnabled() const
{
    return m_ipSplitTunnelingController->isRuSubnetsBypassEnabled();
}

bool IpSplitTunnelingUiController::isRuSubnetsUpdating() const
{
    return m_ipSplitTunnelingController->isRuSubnetsUpdating();
}

QString IpSplitTunnelingUiController::ruSubnetsStatus() const
{
    if (m_ipSplitTunnelingController->isRuSubnetsUpdating()) {
        return tr("Updating the list...");
    }

    const QDateTime updatedAt = m_ipSplitTunnelingController->ruSubnetsUpdatedAt();
    if (!updatedAt.isValid() || m_ipSplitTunnelingController->ruSubnetsCount() == 0) {
        return tr("The list has not been downloaded yet");
    }

    return tr("%1 networks, updated %2, refreshed weekly")
            .arg(m_ipSplitTunnelingController->ruSubnetsCount())
            .arg(QLocale().toString(updatedAt.toLocalTime(), QLocale::ShortFormat));
}

int IpSplitTunnelingUiController::getRouteMode() const
{
    return static_cast<int>(m_ipSplitTunnelingController->getRouteMode());
}

bool IpSplitTunnelingUiController::isSplitTunnelingEnabled() const
{
    return m_ipSplitTunnelingController->isSplitTunnelingEnabled();
}

void IpSplitTunnelingUiController::updateModel()
{
    m_ipSplitTunnelingModel->updateModel(m_ipSplitTunnelingController->getCurrentSites());
}

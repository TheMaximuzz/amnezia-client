#include "windowsSplitTunnelingUiController.h"

#include <QDir>
#include <QFileInfo>

WindowsSplitTunnelingUiController::WindowsSplitTunnelingUiController(AppSplitTunnelingController *appSplitTunnelingController,
                                                                     IpSplitTunnelingController *ipSplitTunnelingController,
                                                                     IpSplitTunnelingModel *tunneledSitesModel,
                                                                     IpSplitTunnelingModel *excludedSitesModel,
                                                                     RunningProcessesModel *runningProcessesModel, QObject *parent)
    : QObject(parent),
      m_appSplitTunnelingController(appSplitTunnelingController),
      m_ipSplitTunnelingController(ipSplitTunnelingController),
      m_tunneledSitesModel(tunneledSitesModel),
      m_excludedSitesModel(excludedSitesModel),
      m_runningProcessesModel(runningProcessesModel)
{
    updateModels();
}

void WindowsSplitTunnelingUiController::addApp(const QString &appPath)
{
    const QString path = QDir::toNativeSeparators(QDir::cleanPath(appPath.trimmed()));
    if (path.isEmpty()) {
        return;
    }

    if (m_appSplitTunnelingController->getRouteMode() != AppsRouteMode::VpnAllExceptApps) {
        m_appSplitTunnelingController->setRouteMode(AppsRouteMode::VpnAllExceptApps);
    }

    const amnezia::InstalledAppInfo appInfo { QFileInfo(path).fileName(), "", path };
    if (!m_appSplitTunnelingController->addApp(appInfo)) {
        emit errorOccurred(tr("The application has already been added"));
        return;
    }

    syncAppsSplitTunneling();
    emit finished(tr("Added: %1").arg(path));
}

void WindowsSplitTunnelingUiController::addAppFolder(const QUrl &folderUrl)
{
    addApp(folderUrl.toLocalFile());
}

void WindowsSplitTunnelingUiController::addProcess(const QString &processPath, bool wholeFolder)
{
    const QString folder = QFileInfo(processPath).absolutePath();
    const QString systemRoot = QDir::fromNativeSeparators(qEnvironmentVariable("SystemRoot"));
    const bool isSystemFolder = !systemRoot.isEmpty() && folder.startsWith(systemRoot, Qt::CaseInsensitive);

    if (wholeFolder && isSystemFolder) {
        emit errorOccurred(tr("A Windows system folder can't be excluded as a whole, only this program was added"));
    }
    addApp(wholeFolder && !isSystemFolder ? folder : processPath);
}

void WindowsSplitTunnelingUiController::moveApp(int index, int offset)
{
    m_appSplitTunnelingController->moveApp(index, index + offset);
}

void WindowsSplitTunnelingUiController::removeApp(int index)
{
    m_appSplitTunnelingController->removeApp(index);
    syncAppsSplitTunneling();
}

void WindowsSplitTunnelingUiController::addTunneledSite(const QString &site)
{
    addSite(RouteMode::VpnOnlyForwardSites, site);
}

void WindowsSplitTunnelingUiController::removeTunneledSite(int index)
{
    removeSite(RouteMode::VpnOnlyForwardSites, m_tunneledSitesModel, index);
}

void WindowsSplitTunnelingUiController::addExcludedSite(const QString &site)
{
    addSite(RouteMode::VpnAllExceptSites, site);
}

void WindowsSplitTunnelingUiController::removeExcludedSite(int index)
{
    removeSite(RouteMode::VpnAllExceptSites, m_excludedSitesModel, index);
}

void WindowsSplitTunnelingUiController::applySiteLists()
{
    m_ipSplitTunnelingController->applyForwardAndExceptLists();
}

void WindowsSplitTunnelingUiController::refreshProcesses(bool allUsers)
{
    m_runningProcessesModel->refresh(allUsers);
}

void WindowsSplitTunnelingUiController::updateModels()
{
    m_tunneledSitesModel->updateModel(m_ipSplitTunnelingController->sitesForMode(RouteMode::VpnOnlyForwardSites));
    m_excludedSitesModel->updateModel(m_ipSplitTunnelingController->sitesForMode(RouteMode::VpnAllExceptSites));
}

void WindowsSplitTunnelingUiController::addSite(RouteMode mode, const QString &site)
{
    if (!m_ipSplitTunnelingController->addSiteToMode(mode, site)) {
        emit errorOccurred(tr("Enter a valid address, network or domain that is not in the list yet"));
    }
}

void WindowsSplitTunnelingUiController::removeSite(RouteMode mode, IpSplitTunnelingModel *model, int index)
{
    const QString site = model->data(model->index(index), IpSplitTunnelingModel::UrlRole).toString();
    if (!site.isEmpty()) {
        m_ipSplitTunnelingController->removeSiteFromMode(mode, site);
    }
}

void WindowsSplitTunnelingUiController::syncAppsSplitTunneling()
{
    m_appSplitTunnelingController->toggleSplitTunneling(!m_appSplitTunnelingController->getApps().isEmpty());
}

#ifndef WINDOWSSPLITTUNNELINGUICONTROLLER_H
#define WINDOWSSPLITTUNNELINGUICONTROLLER_H

#include <QObject>
#include <QUrl>

#include "core/controllers/appSplitTunnelingController.h"
#include "core/controllers/ipSplitTunnelingController.h"
#include "ui/models/ipSplitTunnelingModel.h"
#include "ui/models/runningProcessesModel.h"

class WindowsSplitTunnelingUiController : public QObject
{
    Q_OBJECT

public:
    WindowsSplitTunnelingUiController(AppSplitTunnelingController *appSplitTunnelingController,
                                      IpSplitTunnelingController *ipSplitTunnelingController,
                                      IpSplitTunnelingModel *tunneledSitesModel,
                                      IpSplitTunnelingModel *excludedSitesModel, RunningProcessesModel *runningProcessesModel,
                                      QObject *parent = nullptr);

public slots:
    void addApp(const QString &appPath);
    void addAppFolder(const QUrl &folderUrl);
    void addProcess(const QString &processPath, bool wholeFolder);
    void moveApp(int index, int offset);
    void removeApp(int index);

    void addTunneledSite(const QString &site);
    void removeTunneledSite(int index);
    void addExcludedSite(const QString &site);
    void removeExcludedSite(int index);
    void applySiteLists();

    void refreshProcesses(bool allUsers);
    void updateModels();

signals:
    void finished(const QString &message);
    void errorOccurred(const QString &errorMessage);

private:
    void addSite(RouteMode mode, const QString &site);
    void removeSite(RouteMode mode, IpSplitTunnelingModel *model, int index);
    void syncAppsSplitTunneling();

    AppSplitTunnelingController *m_appSplitTunnelingController;
    IpSplitTunnelingController *m_ipSplitTunnelingController;
    IpSplitTunnelingModel *m_tunneledSitesModel;
    IpSplitTunnelingModel *m_excludedSitesModel;
    RunningProcessesModel *m_runningProcessesModel;
};

#endif // WINDOWSSPLITTUNNELINGUICONTROLLER_H

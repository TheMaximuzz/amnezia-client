#ifndef RUNNINGPROCESSESMODEL_H
#define RUNNINGPROCESSESMODEL_H

#include <QAbstractListModel>
#include <QVector>

class RunningProcessesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        UserRole,
        PathRole
    };

    struct ProcessInfo
    {
        QString name;
        QString user;
        QString path;
    };

    explicit RunningProcessesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void refresh(bool allUsers);

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    static QVector<ProcessInfo> listProcesses(bool allUsers);

    QVector<ProcessInfo> m_processes;
};

#endif // RUNNINGPROCESSESMODEL_H

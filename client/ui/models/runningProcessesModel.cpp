#include "runningProcessesModel.h"

#include <QSet>
#include <algorithm>

#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
#include <memory>
#endif

namespace
{
#ifdef Q_OS_WIN
    struct HandleCloser
    {
        void operator()(HANDLE handle) const
        {
            if (handle && handle != INVALID_HANDLE_VALUE) {
                CloseHandle(handle);
            }
        }
    };
    using UniqueHandle = std::unique_ptr<void, HandleCloser>;

    QByteArray tokenUser(HANDLE process)
    {
        HANDLE rawToken = nullptr;
        if (!OpenProcessToken(process, TOKEN_QUERY, &rawToken)) {
            return {};
        }
        UniqueHandle token(rawToken);

        DWORD size = 0;
        GetTokenInformation(token.get(), TokenUser, nullptr, 0, &size);
        if (size == 0) {
            return {};
        }

        QByteArray buffer(static_cast<int>(size), 0);
        if (!GetTokenInformation(token.get(), TokenUser, buffer.data(), size, &size)) {
            return {};
        }
        return buffer;
    }

    PSID sidOf(const QByteArray &tokenUserBuffer)
    {
        if (tokenUserBuffer.isEmpty()) {
            return nullptr;
        }
        return reinterpret_cast<const TOKEN_USER *>(tokenUserBuffer.constData())->User.Sid;
    }

    QString accountName(PSID sid)
    {
        if (!sid) {
            return {};
        }

        wchar_t name[256];
        wchar_t domain[256];
        DWORD nameSize = 256;
        DWORD domainSize = 256;
        SID_NAME_USE use;
        if (!LookupAccountSidW(nullptr, sid, name, &nameSize, domain, &domainSize, &use)) {
            return {};
        }
        return QString::fromWCharArray(name, static_cast<int>(nameSize));
    }

    QString imagePath(HANDLE process)
    {
        wchar_t path[MAX_PATH * 4];
        DWORD size = MAX_PATH * 4;
        if (!QueryFullProcessImageNameW(process, 0, path, &size)) {
            return {};
        }
        return QString::fromWCharArray(path, static_cast<int>(size));
    }
#endif
}

RunningProcessesModel::RunningProcessesModel(QObject *parent) : QAbstractListModel(parent)
{
}

int RunningProcessesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_processes.size();
}

QVariant RunningProcessesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_processes.size()) {
        return QVariant();
    }

    const ProcessInfo &process = m_processes.at(index.row());
    switch (role) {
    case NameRole: return process.name;
    case UserRole: return process.user;
    case PathRole: return process.path;
    default: return QVariant();
    }
}

void RunningProcessesModel::refresh(bool allUsers)
{
    beginResetModel();
    m_processes = listProcesses(allUsers);
    endResetModel();
}

QHash<int, QByteArray> RunningProcessesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "processName";
    roles[UserRole] = "processUser";
    roles[PathRole] = "processPath";
    return roles;
}

QVector<RunningProcessesModel::ProcessInfo> RunningProcessesModel::listProcesses(bool allUsers)
{
    QVector<ProcessInfo> processes;

#ifdef Q_OS_WIN
    const QByteArray currentUser = tokenUser(GetCurrentProcess());

    UniqueHandle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (snapshot.get() == INVALID_HANDLE_VALUE) {
        return processes;
    }

    PROCESSENTRY32W entry {};
    entry.dwSize = sizeof(entry);
    QSet<QString> seenPaths;

    for (bool hasEntry = Process32FirstW(snapshot.get(), &entry); hasEntry; hasEntry = Process32NextW(snapshot.get(), &entry)) {
        UniqueHandle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ProcessID));
        if (!process) {
            continue;
        }

        const QString path = imagePath(process.get());
        if (path.isEmpty() || seenPaths.contains(path.toLower())) {
            continue;
        }

        const QByteArray owner = tokenUser(process.get());
        const bool isCurrentUser = sidOf(owner) && sidOf(currentUser) && EqualSid(sidOf(owner), sidOf(currentUser));
        if (!allUsers && !isCurrentUser) {
            continue;
        }

        seenPaths.insert(path.toLower());
        processes.append({ QString::fromWCharArray(entry.szExeFile), accountName(sidOf(owner)), path });
    }
#else
    Q_UNUSED(allUsers)
#endif

    std::sort(processes.begin(), processes.end(), [](const ProcessInfo &left, const ProcessInfo &right) {
        return left.name.compare(right.name, Qt::CaseInsensitive) < 0;
    });
    return processes;
}

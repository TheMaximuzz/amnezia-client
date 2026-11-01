#ifndef LOCALINTERFACEUICONTROLLER_H
#define LOCALINTERFACEUICONTROLLER_H

#include <QObject>
#include <QVariantMap>

#include "core/controllers/localInterfaceController.h"

class LocalInterfaceUiController : public QObject
{
    Q_OBJECT

public:
    explicit LocalInterfaceUiController(LocalInterfaceController *localInterfaceController, QObject *parent = nullptr);

public slots:
    QVariantMap load() const;
    void save(const QVariantMap &values);
    QString generatePrivateKey() const;
    QString publicKey(const QString &privateKey) const;

signals:
    void finished(const QString &message);
    void errorOccurred(const QString &errorMessage);

private:
    LocalInterfaceController *m_localInterfaceController;
};

#endif // LOCALINTERFACEUICONTROLLER_H

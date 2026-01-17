/*
 * Copyright (C) 2026 Chupligin Sergey <neochapay@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef RESOURCECLIENT_H
#define RESOURCECLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <qdbuscontext.h>

/**
 * Represents one OHM client.
 * One client == one D-Bus connection (service name).
 */
class ResourceClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(int clientType READ clientType WRITE setClientType NOTIFY clientTypeChanged FINAL)
    Q_PROPERTY(uint clientID READ clientID WRITE setClientID NOTIFY clientIDChanged FINAL)
    Q_PROPERTY(uint clientReqqno READ clientReqqno FINAL)
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName NOTIFY serviceNameChanged FINAL)
    Q_PROPERTY(QString objectPath READ objectPath WRITE setObjectPath NOTIFY objectPathChanged FINAL)

public:
    explicit ResourceClient(QObject* parent = nullptr);

    // identity
    void setPriority(int priority);
    int priority() const { return m_priority; }

    QString objectPath() const { return m_objectPath; }
    void setObjectPath(const QString& path) { m_objectPath = path; }

    // resources
    const QStringList& resources() const { return m_resources; }
    bool hasResource(const QString& resource) const;

    // resource lifecycle (called by ResourceManager)
    void addResource(const QString& resource);
    void removeResource(const QString& resource);

    // notifications (mapped to DBus in adaptor)
    void notifyGranted(const QString& resource);
    void notifyLost(const QString& resource);
    void notifyDenied(const QString& resource);

    int clientType() const;
    void setClientType(int newClientType);

    uint clientID() const;
    void setClientID(uint newClientID);

    uint clientReqqno();

    QString serviceName() const;
    void setServiceName(const QString& newServiceName);

signals:
    void notify(const QString& event,
        const QString& resource);

    void clientTypeChanged();
    void clientIDChanged();
    void clientReqqnoChanged();
    void objectPathChanged();

    void status(int rtype, uint id, uint reqno, int status);

    void serviceNameChanged();

private:
    int m_priority;
    QStringList m_resources;
    QString m_objectPath;
    int m_clientType;
    uint m_clientID;
    uint m_clientReqqno;
    QString m_serviceName;
};

#endif // RESOURCECLIENT_H

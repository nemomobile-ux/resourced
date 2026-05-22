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

    uint pendingReqno() const { return m_pendingReqno; }
    void setPendingReqno(uint reqno) { m_pendingReqno = reqno; }
    void clearPendingReqno() { m_pendingReqno = 0; }
    bool hasPendingRequest() const { return m_pendingReqno != 0; }

    /*For update*/
    uint mandatory() const { return m_mandatory; }
    void setMandatory(uint m) { m_mandatory = m; }
    uint optional() const { return m_optional; }
    void setOptional(uint o) { m_optional = o; }
    uint share() const { return m_share; }
    void setShare(uint s) { m_share = s; }
    uint mask() const { return m_mask; }
    void setMask(uint m) { m_mask = m; }
    QString klass() const { return m_klass; }
    void setKlass(const QString& k) { m_klass = k; }
    QString mode() const { return m_mode; }
    void setMode(const QString& m) { m_mode = m; }

    uint granted() const { return m_granted; }
    void setGranted(uint g) { m_granted = g; }

    /*For audio*/
    void setAudioSpec(const QString& group, const QString& appId, const QString& property,
        const QString& method, const QString& pattern) {
        m_audioGroup = group; m_audioAppId = appId; m_audioProperty = property;
        m_audioMethod = method; m_audioPattern = pattern;
    }

    /*For video*/

    void setVideoPid(uint pid) { m_videoPid = pid; }

    void syncResourcesFromMask(uint mask);

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
    uint m_pendingReqno = 0;
    QStringList m_resources;
    QString m_objectPath;
    int m_clientType;
    uint m_clientID;
    uint m_clientReqqno;
    QString m_serviceName;

    uint m_mandatory = 0, m_optional = 0, m_share = 0, m_mask = 0;
    QString m_klass, m_mode;
    QString m_audioGroup, m_audioAppId, m_audioProperty, m_audioMethod, m_audioPattern;
    uint m_videoPid = 0;
    uint m_granted = 0;
};

#endif // RESOURCECLIENT_H

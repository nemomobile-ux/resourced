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

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QDBusContext>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QMap>
#include <QObject>
#include <QStringList>

class ResourceClient;
class SecurityPolicy;
class PriorityPolicy;

/**
 * Core resource manager.
 * NO DBus code here.
 */
class ResourceManager : public QObject, protected QDBusContext {
    Q_OBJECT

public:
    explicit ResourceManager(QObject* parent = nullptr);

    // client lifecycle
    ResourceClient* createClient(const QDBusMessage& message,
        int priority);
    void destroyClient(ResourceClient* client);
    ResourceClient* findClientById(uint id) const;

    QList<ResourceClient*> clients() const { return m_clients; }

    // resource management
    void requestResources(ResourceClient* client,
        const QStringList& resources);
    void releaseAll(ResourceClient* client);

    // queries
    bool isOwner(const QString& resource,
        const ResourceClient* client) const;

    void emitGranted(ResourceClient* client);
    void reevaluateClient(ResourceClient* client, uint reqno);

    QDBusMessage getMessage() { return message(); }

signals:
    void grantResource(ResourceClient* client, uint reqno, uint mask);
    void adviceResource(ResourceClient* client, uint reqno, uint mask);
    void clientReleased(ResourceClient* client);
    void audioSpecChanged(ResourceClient* client);
    void videoSpecChanged(ResourceClient* client);

private:
    void grant(ResourceClient* client,
        const QString& resource);
    void preempt(ResourceClient* oldClient,
        ResourceClient* newClient,
        const QString& resource);

    // resource → owner
    QMap<QString, ResourceClient*> m_resourceOwners;

    // active clients
    QList<ResourceClient*> m_clients;

    SecurityPolicy* m_security;
    PriorityPolicy* m_priority;

    uint resourceMask(const QString& resource);
};

#endif // RESOURCEMANAGER_H

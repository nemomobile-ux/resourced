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

#include "resourcemanager.h"
#include "resourceclient.h"
#include <policy/prioritypolicy.h>
#include <policy/securitypolicy.h>
#include <util/logger.h>

#include <QDBusConnection>
#include <QDBusMessage>

ResourceManager::ResourceManager(QObject* parent)
    : QObject(parent)
    , m_security(new SecurityPolicy(this))
    , m_priority(new PriorityPolicy(this))
{
}

ResourceClient* ResourceManager::createClient(const QDBusMessage& message, int priority)
{
    ResourceClient* client = new ResourceClient(this);

    m_clients.append(client);

    qCDebug(lcResourceDaemonCoreLog) << "Client created:" << message.service();
    qCDebug(lcResourceDaemonCoreLog) << "priority:" << QString::number(priority);

    return client;
}

void ResourceManager::destroyClient(ResourceClient* client)
{
    if (!client)
        return;

    qCDebug(lcResourceDaemonCoreLog) << "Client destroyed" << client->objectPath();

    releaseAll(client);
    m_clients.removeAll(client);
    client->deleteLater();
}

void ResourceManager::requestResources(ResourceClient* client,
    const QStringList& resources)
{
    if (!client)
        return;

    for (const QString& res : resources) {

        auto* owner = m_resourceOwners.value(res, nullptr);

        // free resource
        if (!owner) {
            grant(client, res);
            continue;
        }

        // already owns
        if (owner == client)
            continue;

        // PREEMPTION DECISION
        if (m_priority->canPreempt(client, owner, res)) {
            preempt(owner, client, res);
        } else {
            client->notifyDenied(res);
        }
    }
}

void ResourceManager::releaseAll(ResourceClient* client)
{
    if (!client)
        return;

    const auto resources = client->resources();

    for (const QString& res : resources) {
        m_resourceOwners.remove(res);
        client->removeResource(res);
    }
}

bool ResourceManager::isOwner(const QString& resource,
    const ResourceClient* client) const
{
    return m_resourceOwners.value(resource) == client;
}

void ResourceManager::emitGranted(ResourceClient* client)
{
    QDBusMessage sig = QDBusMessage::createSignal(
        client->objectPath(),
        "org.maemo.resource.client",
        "granted");

    sig << 9
        << client->clientType()
        << (uint)client->clientID()
        << (uint)client->clientReqqno()
        << (uint)0
        << "ok";

    QDBusConnection::systemBus().send(sig);

    qCDebug(lcResourceDaemonCoreLog) << "Granted resource"
                    << "rtype=" << client->clientType()
                    << "id=" << client->clientID()
                    << "reqno=" << client->clientReqqno()
                    << "to" << client->objectPath();

    QDBusMessage status = QDBusMessage::createSignal(
        client->objectPath(),
        "org.maemo.resource.client",
        "status");

    status << 9
           << client->clientType() // rtype (int32)
           << (uint)client->clientID() // id (uint32)
           << (uint)client->clientReqqno() // reqno (uint32)
           << (uint)1; // status = ACTIVE

    QDBusConnection::systemBus().send(status);
}

/* private */

void ResourceManager::grant(ResourceClient* client,
    const QString& resource)
{
    m_resourceOwners.insert(resource, client);
    client->addResource(resource);

    client->notifyGranted(resource);

    qCDebug(lcResourceDaemonCoreLog) << "Granted" + resource + " to " + client->objectPath();
}

void ResourceManager::preempt(ResourceClient* oldClient,
    ResourceClient* newClient,
    const QString& resource)
{
    qCDebug(lcResourceDaemonCoreLog) <<  "Preempting" + resource + " from " + oldClient->objectPath() + " to " + newClient->objectPath();

    oldClient->removeResource(resource);
    oldClient->notifyLost(resource);

    m_resourceOwners.remove(resource);

    grant(newClient, resource);
}

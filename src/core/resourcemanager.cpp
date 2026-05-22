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

    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Client created:" << message.service();
    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "priority:" << QString::number(priority);

    return client;
}

void ResourceManager::destroyClient(ResourceClient* client)
{
    if (!client)
        return;

    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Client destroyed" << client->objectPath();

    releaseAll(client);
    m_clients.removeAll(client);
    client->deleteLater();
}

ResourceClient *ResourceManager::findClientById(uint id) const
{
    for (ResourceClient* client : m_clients) {
        if (client->clientID() == id)
            return client;
    }
    return nullptr;
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

    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Granted resource"
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

void ResourceManager::reevaluateClient(ResourceClient *client, uint reqno)
{
    // Safety checks
    if (!client) {
        qCWarning(lcResourceDaemonCoreLog) << "reevaluateClient: null client";
        return;
    }

    uint requested = client->mandatory() | client->optional();
    uint currentGranted = client->granted();

    uint newGranted = requested;

    if (newGranted == currentGranted) {
        qCDebug(lcResourceDaemonCoreLog) << "reevaluateClient: no change for client"
                                         << client->objectPath() << "granted stays" << newGranted;
        return;
    }

    client->setGranted(newGranted);

    client->syncResourcesFromMask(newGranted);
    client->setGranted(newGranted);

    emit grantResource(client, reqno, newGranted);

    qCDebug(lcResourceDaemonCoreLog) << "reevaluateClient: client" << client->objectPath()
                                     << "granted changed from" << currentGranted
                                     << "to" << newGranted << "(reqno" << reqno << ")";
}

/* private */

void ResourceManager::grant(ResourceClient* client,
    const QString& resource)
{
    m_resourceOwners.insert(resource, client);
    client->addResource(resource);

    uint mask = resourceMask(resource);
    uint reqno = client->pendingReqno();

    emit grantResource(client, reqno, mask);

    client->clearPendingReqno();
}

void ResourceManager::preempt(ResourceClient* oldClient,
    ResourceClient* newClient,
    const QString& resource)
{
    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Preempting" + resource + " from " + oldClient->objectPath() + " to " + newClient->objectPath();

    oldClient->removeResource(resource);
    oldClient->notifyLost(resource);

    m_resourceOwners.remove(resource);

    grant(newClient, resource);
}

uint ResourceManager::resourceMask(const QString &resource)
{
    if (resource == "AudioPlayback")  return 1 << 0; // 1
    if (resource == "VideoPlayback")  return 1 << 1; // 2
    if (resource == "AudioCapture")   return 1 << 2; // 4
    if (resource == "Alarm")          return 1 << 3; // 8
    if (resource == "VoiceCall")      return 1 << 4; // 16
    if (resource == "HardwareKeys")   return 1 << 5; // 32
    if (resource == "TouchInput")     return 1 << 6; // 64
    if (resource == "Location")       return 1 << 7; // 128
    if (resource == "Network")        return 1 << 8; // 256
    if (resource == "Display")        return 1 << 9; // 512
    return 0;
}

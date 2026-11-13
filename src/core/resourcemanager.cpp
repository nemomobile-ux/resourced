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
    client->setPriority(priority);

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
    QDBusConnection::systemBus().unregisterObject(client->objectPath());
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

        auto& owners = m_resourceOwners[res];

        // already owns
        if (owners.contains(client))
            continue;

        // free resource
        if (owners.isEmpty()) {
            grant(client, res);
            continue;
        }

        uint bit = resourceMask(res);

        // shared resource: grant without preemption if all current owners share
        bool shareable = !owners.isEmpty();
        for (ResourceClient* owner : owners) {
            if (!((owner->share() & bit) && (client->share() & bit) &&
                  (owner->mask() & bit) && (client->mask() & bit))) {
                shareable = false;
                break;
            }
        }

        if (shareable) {
            grant(client, res);
            continue;
        }

        // preemption decision: all owners must be preemptable
        bool canPreemptAll = true;
        for (ResourceClient* owner : owners) {
            if (!m_priority->canPreempt(client, owner, res)) {
                canPreemptAll = false;
                break;
            }
        }

        if (canPreemptAll) {
            for (ResourceClient* owner : owners) {
                owner->removeResource(res);
                owner->notifyLost(res);
            }
            owners.clear();
            m_resourceOwners.remove(res);
            grant(client, res);
        } else {
            client->notifyDenied(res);
            emit adviceResource(client, client->pendingReqno(), resourceMask(res));
        }
    }

    client->clearPendingReqno();
}

void ResourceManager::releaseAll(ResourceClient* client)
{
    if (!client)
        return;

    const auto resources = client->resources();

    for (const QString& res : resources) {
        auto& owners = m_resourceOwners[res];
        owners.removeAll(client);
        if (owners.isEmpty())
            m_resourceOwners.remove(res);

        client->removeResource(res);
        client->notifyLost(res);
    }
}

bool ResourceManager::isOwner(const QString& resource,
    const ResourceClient* client) const
{
    return m_resourceOwners.value(resource).contains(const_cast<ResourceClient*>(client));
}

bool ResourceManager::isAllowedSender(const QString& sender) const
{
    return m_security->isAllowedSender(sender);
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

    auto bitToName = [](uint bit) -> QString {
        switch (bit) {
        case 1:   return "AudioPlayback";
        case 2:   return "VideoPlayback";
        case 4:   return "AudioCapture";
        case 8:   return "Alarm";
        case 16:  return "VoiceCall";
        case 32:  return "HardwareKeys";
        case 64:  return "TouchInput";
        case 128: return "Location";
        case 256: return "Network";
        case 512: return "Display";
        default:  return QString();
        }
    };

    if (requested == currentGranted) {
        qCDebug(lcResourceDaemonCoreLog) << "reevaluateClient: no change for client"
                                         << client->objectPath() << "granted stays" << requested;
        return;
    }

    uint removed = currentGranted & ~requested;
    for (uint bit = 1; bit <= 512; bit <<= 1) {
        if (!(removed & bit))
            continue;
        QString name = bitToName(bit);
        if (name.isEmpty())
            continue;
        auto& owners = m_resourceOwners[name];
        owners.removeAll(client);
        if (owners.isEmpty())
            m_resourceOwners.remove(name);
        client->removeResource(name);
        client->notifyLost(name);
    }

    uint added = requested & ~currentGranted;
    uint grantedAdded = 0;
    for (uint bit = 1; bit <= 512; bit <<= 1) {
        if (!(added & bit))
            continue;
        QString name = bitToName(bit);
        if (name.isEmpty())
            continue;

        auto& owners = m_resourceOwners[name];

        if (owners.isEmpty()) {
            if (!owners.contains(client))
                owners.append(client);
            client->addResource(name);
            grantedAdded |= bit;
            continue;
        }

        if (owners.contains(client)) {
            grantedAdded |= bit;
            continue;
        }

        bool shareable = true;
        for (ResourceClient* owner : owners) {
            if (!((owner->share() & bit) && (client->share() & bit) &&
                  (owner->mask() & bit) && (client->mask() & bit))) {
                shareable = false;
                break;
            }
        }

        if (shareable) {
            owners.append(client);
            client->addResource(name);
            grantedAdded |= bit;
            continue;
        }

        bool canPreemptAll = true;
        for (ResourceClient* owner : owners) {
            if (!m_priority->canPreempt(client, owner, name)) {
                canPreemptAll = false;
                break;
            }
        }

        if (canPreemptAll) {
            for (ResourceClient* owner : owners) {
                owner->removeResource(name);
                owner->notifyLost(name);
            }
            m_resourceOwners.remove(name);
            auto& newOwners = m_resourceOwners[name];
            newOwners.append(client);
            client->addResource(name);
            grantedAdded |= bit;
        } else {
            client->notifyDenied(name);
            emit adviceResource(client, reqno, bit);
        }
    }

    uint newGranted = (currentGranted & requested) | grantedAdded;
    client->setGranted(newGranted);
    client->syncResourcesFromMask(newGranted);

    if (newGranted != currentGranted)
        emit grantResource(client, reqno, newGranted);

    qCDebug(lcResourceDaemonCoreLog) << "reevaluateClient: client" << client->objectPath()
                                     << "granted changed from" << currentGranted
                                     << "to" << newGranted << "(reqno" << reqno << ")";
}

/* private */

void ResourceManager::grant(ResourceClient* client,
    const QString& resource)
{
    auto& owners = m_resourceOwners[resource];
    if (!owners.contains(client))
        owners.append(client);

    client->addResource(resource);

    uint mask = resourceMask(resource);
    uint reqno = client->pendingReqno();

    emit grantResource(client, reqno, mask);
}

void ResourceManager::preempt(ResourceClient* oldClient,
    ResourceClient* newClient,
    const QString& resource)
{
    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Preempting" + resource + " from " + oldClient->objectPath() + " to " + newClient->objectPath();

    auto& owners = m_resourceOwners[resource];
    owners.removeAll(oldClient);
    if (owners.isEmpty())
        m_resourceOwners.remove(resource);

    oldClient->removeResource(resource);
    oldClient->notifyLost(resource);

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

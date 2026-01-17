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

#include "resourceclient.h"

ResourceClient::ResourceClient(
    QObject* parent)
    : QObject(parent)
    , m_clientID(-1)
    , m_clientReqqno(0)
{
}

void ResourceClient::setPriority(int priority)
{
    m_priority = priority;
}

bool ResourceClient::hasResource(const QString& resource) const
{
    return m_resources.contains(resource);
}

void ResourceClient::addResource(const QString& resource)
{
    if (m_resources.contains(resource))
        return;

    m_resources.append(resource);
}

void ResourceClient::removeResource(const QString& resource)
{
    m_resources.removeAll(resource);
}

/* notifications */

void ResourceClient::notifyGranted(const QString& resource)
{
    emit notify(QStringLiteral("granted"), resource);
}

void ResourceClient::notifyLost(const QString& resource)
{
    emit notify(QStringLiteral("lost"), resource);
}

void ResourceClient::notifyDenied(const QString& resource)
{
    emit notify(QStringLiteral("denied"), resource);
}

int ResourceClient::clientType() const
{
    return m_clientType;
}

void ResourceClient::setClientType(int newClientType)
{
    if (m_clientType == newClientType)
        return;
    m_clientType = newClientType;
    emit clientTypeChanged();
}

uint ResourceClient::clientID() const
{
    return m_clientID;
}

void ResourceClient::setClientID(uint newClientID)
{
    m_clientID = newClientID;
}

uint ResourceClient::clientReqqno()
{
    m_clientReqqno++;
    return m_clientReqqno;
}

QString ResourceClient::serviceName() const
{
    return m_serviceName;
}

void ResourceClient::setServiceName(const QString& newServiceName)
{
    if (m_serviceName == newServiceName)
        return;
    m_serviceName = newServiceName;
    emit serviceNameChanged();
}

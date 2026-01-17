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

#ifndef MANAGERADAPTOR_H
#define MANAGERADAPTOR_H

#include "core/resourcemanager.h"
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QDBusVirtualObject>
#include <QObject>

/**
 * DBus adaptor for org.maemo.resource.manager
 * Exports methods to system bus.
 */
class ManagerAdaptor : public QDBusVirtualObject {
    Q_OBJECT
public:
    explicit ManagerAdaptor(ResourceManager* parent);
    ~ManagerAdaptor() override;
    ResourceManager* parent() const;

    QString introspect(const QString& path) const override;
    bool handleMessage(const QDBusMessage& message, const QDBusConnection& connection) override;

Q_SIGNALS: // SIGNALS
    void ClientRegistered(const QString& client_name, const QDBusObjectPath& client_path);
    void ClientUnregistered(const QDBusObjectPath& client_path);

private:
    void registerClient(const QDBusMessage& message, const QDBusConnection& connection);
    void unregisterClient(const QDBusObjectPath& path);
    void acquireClient(const QDBusMessage& message, const QDBusConnection& connection);

    uint m_clientsCount;

    void printDebug(const QDBusMessage& message);
};

#endif // MANAGERADAPTOR_H

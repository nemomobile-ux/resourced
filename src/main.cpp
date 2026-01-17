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

#include <QCoreApplication>
#include <QDBusConnection>

#include "core/resourcemanager.h"
#include "dbus/manageradaptor.h"
#include "util/logger.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("resourced");
    app.setOrganizationName("org.nemomobile");

    qCDebug(lcResourceDaemonCoreLog) <<  "Starting resourced daemon...";
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        qCWarning(lcResourceDaemonCoreLog) << "Cannot connect to system D-Bus:" << bus.lastError().message();
        return -1;
    }

    if (!bus.registerService("org.maemo.resource.manager")) {
        qCWarning(lcResourceDaemonCoreLog) << "Cannot register D-Bus service:" << bus.lastError().message();
        return -1;
    }

    // Core manager
    ResourceManager* manager = new ResourceManager();
    ManagerAdaptor adaptor(manager);

    if (!bus.registerVirtualObject(
            "/org/maemo/resource/manager",
            &adaptor)) {
        qCWarning(lcResourceDaemonCoreLog) << "Failed to register virtual object on system bus";
    }

    qCDebug(lcResourceDaemonCoreLog) << "resourced started, waiting for clients...";
    return app.exec();
}

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
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>

#include "core/resourcemanager.h"
#include "dbus/manageradaptor.h"
#include "util/logger.h"

static QString findConfigFile()
{
    const QStringList candidates = {
        QStringLiteral("/etc/resourced.conf"),
        QStringLiteral("/etc/resourced/resourced.conf"),
        QStringLiteral("/usr/share/resourced/resourced.conf")
    };

    for (const QString& path : candidates) {
        if (QFileInfo::exists(path))
            return path;
    }

    return QString();
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("resourced");
    app.setOrganizationName("org.nemomobile");

    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Starting resourced daemon...";
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        qCWarning(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Cannot connect to system D-Bus:" << bus.lastError().message();
        return -1;
    }

    if (!bus.registerService("org.maemo.resource.manager")) {
        qCWarning(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Cannot register D-Bus service:" << bus.lastError().message();
        return -1;
    }

    // Core manager
    ResourceManager* manager = new ResourceManager();

    QString configFile = findConfigFile();
    if (!configFile.isEmpty()) {
        QSettings settings(configFile, QSettings::IniFormat);
        settings.beginGroup("Security");
        QString allowed = settings.value("AllowedSenders", "org.nemomobile").toString();
        QStringList senders;
        for (const QString& part : allowed.split(QRegularExpression("[,;\\s]+"), Qt::SkipEmptyParts))
            senders << part.trimmed();
        if (!senders.isEmpty())
            manager->setAllowedSenders(senders);
        settings.endGroup();

        settings.beginGroup("Preemption");
        bool preemption = settings.value("EnablePreemption", true).toBool();
        manager->setPreemptionEnabled(preemption);
        settings.endGroup();

        settings.beginGroup("General");
        int defaultPriority = settings.value("DefaultClientPriority", 10).toInt();
        manager->setDefaultClientPriority(defaultPriority);
        settings.endGroup();
    }

    ManagerAdaptor adaptor(manager);

    if (!bus.registerVirtualObject(
            "/org/maemo/resource/manager",
            &adaptor)) {
        qCWarning(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Failed to register virtual object on system bus";
    }

    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "resourced started, waiting for clients...";
    return app.exec();
}
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

#include "clientadaptor.h"
#include "core/resourceclient.h"
#include "util/logger.h"

#include <QDBusMessage>
#include <qdbusconnection.h>

ClientAdaptor::ClientAdaptor(ResourceClient* parent)
    : QDBusVirtualObject(parent)
{
}

ClientAdaptor::~ClientAdaptor()
{
}

QString ClientAdaptor::introspect(const QString& path) const
{
    return R"(
<interface name="org.maemo.resource.client">
    <method name="grant">
        <arg type="i" direction="in"/> <!--    type   -->
        <arg type="u" direction="in"/> <!--     id    -->
        <arg type="u" direction="in"/> <!--    reqno  -->
        <arg type="u" direction="in"/> <!-- resources -->
    </method>
    <method name="advice">
        <arg type="i" direction="in"/> <!--    type   -->
        <arg type="u" direction="in"/> <!--     id    -->
        <arg type="u" direction="in"/> <!--    reqno  -->
        <arg type="u" direction="in"/> <!-- resources -->
    </method>
</interface>
    )";
}

bool ClientAdaptor::handleMessage(const QDBusMessage& message, const QDBusConnection& connection)
{
    if (message.type() != QDBusMessage::MethodCallMessage)
        return false;

    if (message.interface() == QLatin1String("org.freedesktop.DBus.Introspectable"))
        return false;

    if (message.interface() != "org.maemo.resource.client")
        return false;

    uint clientId = message.arguments()[1].toUInt();
    uint reqno = message.arguments()[2].toUInt();

    printDebug(message);

    // ---- Method reply ----
    if (message.type() == QDBusMessage::MethodCallMessage) {
        QDBusMessage reply = message.createReply();
        connection.send(reply);
        return true;
    }
    return false;
}

void ClientAdaptor::printDebug(const QDBusMessage& message)
{
    if (message.arguments().count() < 3) {
        qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "==== skip system message ===";
        return;
    }

    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "==== got messsage ==========";
    qCDebug(lcResourceDaemonCoreLog) <<  "Type   : " << message.arguments()[0].toInt();
    qCDebug(lcResourceDaemonCoreLog) <<  "ID     : " << message.arguments()[1].toUInt();
    qCDebug(lcResourceDaemonCoreLog) <<  "Req NO : " << message.arguments()[2].toUInt();
}

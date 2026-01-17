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
    QVariantList replyArgs;
    replyArgs << 5
              << (uint)clientId
              << (uint)reqno
              << (uint)1024;

    QDBusMessage reply = message.createReply(replyArgs);
    connection.send(reply);

    Logger::log(Logger::Debug, "ClientAdaptor", "==== send messsage ==========");
    Logger::log(Logger::Debug, "ClientAdaptor", "Type   : " + replyArgs[0].toInt());
    Logger::log(Logger::Debug, "ClientAdaptor", "ID     : " + replyArgs[1].toUInt());
    Logger::log(Logger::Debug, "ClientAdaptor", "Req NO : " + replyArgs[2].toUInt());

    return true;
}

void ClientAdaptor::printDebug(const QDBusMessage& message)
{
    if (message.arguments().count() < 3) {
        Logger::log(Logger::Debug, "ClientAdaptor", "==== skip system message ===");
        return;
    }

    Logger::log(Logger::Debug, "ClientAdaptor", "==== got messsage ==========");
    Logger::log(Logger::Debug, "ClientAdaptor", "Type   : " + message.arguments()[0].toInt());
    Logger::log(Logger::Debug, "ClientAdaptor", "ID     : " + message.arguments()[1].toUInt());
    Logger::log(Logger::Debug, "ClientAdaptor", "Req NO : " + message.arguments()[2].toUInt());
}

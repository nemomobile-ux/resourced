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

#include "manageradaptor.h"
#include "core/resourceclient.h"
#include "core/resourcemanager.h"
#include "dbus/clientadaptor.h"
#include "util/logger.h"

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>
#include <qdbusconnectioninterface.h>
#include <qfileinfo.h>

ManagerAdaptor::ManagerAdaptor(ResourceManager* parent)
    : QDBusVirtualObject(parent)
    , m_clientsCount(0)
{
}

ManagerAdaptor::~ManagerAdaptor()
{
}

ResourceManager* ManagerAdaptor::parent() const
{
    return static_cast<ResourceManager*>(QObject::parent());
}

QString ManagerAdaptor::introspect(const QString& path) const
{
    return
        R"(
<interface name="org.maemo.resource.manager">
    <method name="register">
        <arg type="i" direction="in"/>   <!-- type -->
        <arg type="u" direction="in"/>   <!-- id -->
        <arg type="u" direction="in"/>   <!-- reqno -->
        <arg type="u" direction="in"/>   <!-- mandatory -->
        <arg type="u" direction="in"/>   <!-- optional -->
        <arg type="u" direction="in"/>   <!-- share -->
        <arg type="u" direction="in"/>   <!-- mask -->
        <arg type="s" direction="in"/>   <!-- klass -->
        <arg type="s" direction="in"/>   <!-- mode -->
        <arg type="u" direction="in"/>   <!-- priority -->
        <arg type="i" direction="out"/>  <!-- rtype -->
        <arg type="u" direction="out"/>  <!-- id_out -->
        <arg type="u" direction="out"/>  <!-- reqno_out -->
        <arg type="i" direction="out"/>  <!-- error -->
        <arg type="s" direction="out"/>  <!-- message -->
    </method>
    <method name=acquire">
        <arg type="i" direction="in"/>
        <arg type="u" direction="in"/>
        <arg type="u" direction="in"/>
        <arg type="u" direction="in"/>
        <arg type="i" direction="out"/>  <!-- rtype -->
        <arg type="u" direction="out"/>  <!-- id_out -->
        <arg type="u" direction="out"/>  <!-- reqno_out -->
        <arg type="i" direction="out"/>  <!-- error -->
        <arg type="s" direction="out"/>  <!-- message -->
    </method>
    <method name="unregister">
        <arg type="i" direction="in"/>
        <arg type="u" direction="in"/>
        <arg type="u" direction="in"/>
        <arg type="i" direction="out"/>  <!-- rtype -->
        <arg type="u" direction="out"/>  <!-- id_out -->
        <arg type="u" direction="out"/>  <!-- reqno_out -->
        <arg type="i" direction="out"/>  <!-- error -->
        <arg type="s" direction="out"/>  <!-- message -->
    </method>
    <method name=release">
        <arg type="i" direction="in"/>
        <arg type="u" direction="in"/>
        <arg type="u" direction="in"/>
        <arg type="i" direction="out"/>  <!-- rtype -->
        <arg type="u" direction="out"/>  <!-- id_out -->
        <arg type="u" direction="out"/>  <!-- reqno_out -->
        <arg type="i" direction="out"/>  <!-- error -->
        <arg type="s" direction="out"/>  <!-- message -->
    </method>
</interface>)";
}

bool ManagerAdaptor::handleMessage(const QDBusMessage& message, const QDBusConnection& connection)
{
    QString member = message.member();
    QString interface = message.interface();

    if (message.interface() == "org.freedesktop.DBus.Introspectable") {
        return false;
    }

    if (message.type() != QDBusMessage::MethodCallMessage)
        return false;

    if (message.interface() != "org.maemo.resource.manager")
        return false;

    printDebug(message);

    if (message.member() == "register") {
        registerClient(message, connection);
        return true;
    }
    if (message.member() == "unregister") {
        return true;
    }

    if (message.member() == "acquire") {
        acquireClient(message, connection);
        return true;
    }
    return false;
}

/**
 * Register a new client.
 * Only allowed senders can register.
 */
void ManagerAdaptor::registerClient(const QDBusMessage& message, const QDBusConnection& connection)
{
    const auto args = message.arguments();

    QVariantList replyArgs;
    ResourceClient* client = nullptr;

    if (args.size() != 10) {
        qCWarning(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Wrong arguments";
        replyArgs << 0 << 0 << 0 << -1 << "Invalid argument count";
    } else {

        // ---- Parse arguments ----
        const int type = args[0].toInt();
        const uint id = args[1].toUInt();
        const uint reqno = args[2].toUInt();
        const uint mandatory = args[3].toUInt();
        const uint optional = args[4].toUInt();
        const uint share = args[5].toUInt();
        const uint mask = args[6].toUInt();
        const QString klass = args[7].toString();
        const QString mode = args[8].toString();
        const uint priority = args[9].toUInt();

        client = parent()->createClient(message, priority);
        client->setClientType(type);
        uint clientId = m_clientsCount + 1;
        m_clientsCount = m_clientsCount + 1;

        // Register object path on DBus
        QString path = QString("/org/maemo/resource/client%1")
                           .arg(clientId);
        client->setClientID(clientId);
        client->setObjectPath(path);
        client->setServiceName(message.service());

        ClientAdaptor* clientAdaptor = new ClientAdaptor(client);
        bool ok = QDBusConnection::systemBus().registerVirtualObject(
            path,
            clientAdaptor);

        if (!ok) {
            qCWarning(lcResourceDaemonCoreLog) << "Cannot register client object" + path;
            replyArgs << 0 << 0 << 0 << -1 << "Cannot register client object";
        } else {
            replyArgs << (int)9
                      << (uint)client->clientID()
                      << (uint)reqno
                      << (uint)0
                      << QStringLiteral("OK");
        }
    }

    qCDebug(lcResourceDaemonCoreLog) << "==== send messsage ==========";
    qCDebug(lcResourceDaemonCoreLog) << "Type   : " << replyArgs[0].toInt();
    qCDebug(lcResourceDaemonCoreLog) << "ID     : " << replyArgs[1].toUInt();
    qCDebug(lcResourceDaemonCoreLog) << "Req NO : " << replyArgs[2].toUInt();

    QDBusMessage reply = message.createReply(replyArgs);
    connection.call(reply);
}

/**
 * Unregister a client by object path
 */
void ManagerAdaptor::unregisterClient(const QDBusObjectPath& path)
{
    // Lookup client by object path
    ResourceClient* client = nullptr;
    for (auto c : parent()->clients()) {
        if (c->objectPath() == path.path()) {
            client = c;
            break;
        }
    }

    if (!client) {
        qCWarning(lcResourceDaemonCoreLog) <<  "unregisterClient: no such client" + path.path();
        return;
    }

    // Only the owner can unregister
    if (parent()->getMessage().service() != client->objectPath()) {
        qCWarning(lcResourceDaemonCoreLog) <<  "unregisterClient denied for sender" + parent()->getMessage().service();
        return;
    }

    parent()->destroyClient(client);
    qCDebug(lcResourceDaemonCoreLog) << "Client unregistered:" + path.path();
}

void ManagerAdaptor::acquireClient(const QDBusMessage& message, const QDBusConnection& connection)
{
    const auto args = message.arguments();
    const int resourceType = args[0].toInt(); // usually 3 (event)
    const uint rsetId = args[1].toUInt(); // ResourceSet id
    const uint reqno = args[2].toUInt();

    ResourceClient* client = nullptr;
    for (auto c : parent()->clients()) {
        if (c->clientID() == rsetId) {
            client = c;
            break;
        }
    }
    if (!client) {
        qCDebug(lcResourceDaemonCoreLog) << "acquireClient: client not found:" << rsetId;
        return;
    }

    QVariantList replyArgs;
    replyArgs << (int)9
              << (uint)client->clientID()
              << (uint)reqno
              << (uint)0
              << QStringLiteral("OK");

    QDBusMessage reply = message.createReply(replyArgs);

    qCDebug(lcResourceDaemonCoreLog) << "==== send messsage ==========";
    qCDebug(lcResourceDaemonCoreLog) << "Type   : " << replyArgs[0].toInt();
    qCDebug(lcResourceDaemonCoreLog) << "ID     : " << replyArgs[1].toUInt();
    qCDebug(lcResourceDaemonCoreLog) << "Req NO : " << replyArgs[2].toUInt();

    connection.send(reply);

    qCDebug(lcResourceDaemonCoreLog) << "ACQUIRE completed for client" + client->objectPath();

    if (client->serviceName().isEmpty()) {
        qCWarning(lcResourceDaemonCoreLog) << "Client serviceName is empty, cannot call grant()";
        return;
    }

    QDBusMessage grant = QDBusMessage::createMethodCall(
        client->serviceName(),
        client->objectPath(), // /org/maemo/resource/clientX
        QStringLiteral("org.maemo.resource.client"),
        QStringLiteral("grant"));

    // grant(int32 rtype, uint32 id, uint32 reqno, uint32 mask)
    grant << (int)5
          << (uint)client->clientID()
          << (uint)reqno
          << (uint)1024; // обычно mask/share

    connection.send(grant);
    qCDebug(lcResourceDaemonCoreLog) << "Sent grant() to client:"
                    << client->objectPath()
                    << "rtype=" << 5
                    << "id=" << client->clientID()
                    << "reqno=" << reqno;
}

void ManagerAdaptor::printDebug(const QDBusMessage& message)
{
    if (message.arguments().count() < 3) {
        qCDebug(lcResourceDaemonCoreLog) << "==== skip system message ===";
        return;
    }

    qCDebug(lcResourceDaemonCoreLog) << "==== got messsage ==========";
    qCDebug(lcResourceDaemonCoreLog) << "Type   : " << message.arguments()[0].toInt()
                                     << "ID     : " << message.arguments()[1].toUInt()
                                     << "Req NO : " << message.arguments()[2].toUInt();
}

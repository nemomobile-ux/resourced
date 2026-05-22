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
    connect(parent, &ResourceManager::grantResource,
        this, &ManagerAdaptor::sendGrant);
    connect(parent, &ResourceManager::adviceResource,
        this, &ManagerAdaptor::sendAdvice);
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

bool ManagerAdaptor::handleMessage(const QDBusMessage& message
    , const QDBusConnection& connection)
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

    if (member == QLatin1String("register")) {
        registerClient(message, connection);
        return true;
    }

    if (member == QLatin1String("acquire")) {
        handleAcquire(message, connection);
        return true;
    }

    if (member == QLatin1String("release")) {
        handleRelease(message, connection);
        return true;
    }

    if (member == QLatin1String("unregister")) {
        handleUnregister(message, connection);
        return true;
    }

    if (member == QLatin1String("update")) {
        handleUpdate(message, connection);
        return true;
    }

    if (member == QLatin1String("audio")) {
        handleAudio(message, connection);
        return true;
    }

    if (member == QLatin1String("video")) {
        handleVideo(message, connection);
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
            qCWarning(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Cannot register client object" + path;
            replyArgs << 0 << 0 << 0 << -1 << "Cannot register client object";
        } else {
            replyArgs << (int)9
                      << (uint)client->clientID()
                      << (uint)reqno
                      << (uint)0
                      << QStringLiteral("OK");
        }
    }

    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "==== send messsage ==========";
    qCDebug(lcResourceDaemonCoreLog) << "Type   : " << replyArgs[0].toInt();
    qCDebug(lcResourceDaemonCoreLog) << "ID     : " << replyArgs[1].toUInt();
    qCDebug(lcResourceDaemonCoreLog) << "Req NO : " << replyArgs[2].toUInt();

    QDBusMessage reply = message.createReply(replyArgs);
    connection.send(reply);
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
        qCWarning(lcResourceDaemonCoreLog) << Q_FUNC_INFO <<  "unregisterClient: no such client" + path.path();
        return;
    }

    // Only the owner can unregister
    if (parent()->getMessage().service() != client->objectPath()) {
        qCWarning(lcResourceDaemonCoreLog) << Q_FUNC_INFO <<  "unregisterClient denied for sender" + parent()->getMessage().service();
        return;
    }

    parent()->destroyClient(client);
    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Client unregistered:" + path.path();
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
        qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "acquireClient: client not found:" << rsetId;
        return;
    }

    QVariantList replyArgs;
    replyArgs << (int)9
              << (uint)client->clientID()
              << (uint)reqno
              << (uint)0
              << QStringLiteral("OK");

    QDBusMessage reply = message.createReply(replyArgs);

    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "==== send messsage ==========";
    qCDebug(lcResourceDaemonCoreLog) << "Type   : " << replyArgs[0].toInt();
    qCDebug(lcResourceDaemonCoreLog) << "ID     : " << replyArgs[1].toUInt();
    qCDebug(lcResourceDaemonCoreLog) << "Req NO : " << replyArgs[2].toUInt();

    connection.send(reply);

    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "ACQUIRE completed for client" + client->objectPath();

    if (client->serviceName().isEmpty()) {
        qCWarning(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "Client serviceName is empty, cannot call grant()";
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
    qCDebug(lcResourceDaemonCoreLog)  << Q_FUNC_INFO << "Sent grant() to client:"
                    << client->objectPath()
                    << "rtype=" << 5
                    << "id=" << client->clientID()
                    << "reqno=" << reqno;
}

void ManagerAdaptor::printDebug(const QDBusMessage& message)
{
    if (message.arguments().count() < 3) {
        qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "==== skip system message ===";
        return;
    }

    qCDebug(lcResourceDaemonCoreLog) << Q_FUNC_INFO << "==== got messsage ==========";
    qCDebug(lcResourceDaemonCoreLog) << "Type   : " << message.arguments()[0].toInt()
                                     << "ID     : " << message.arguments()[1].toUInt()
                                     << "Req NO : " << message.arguments()[2].toUInt();
}

void ManagerAdaptor::handleAcquire(const QDBusMessage &message, const QDBusConnection &connection)
{
    Q_UNUSED(connection);
    const QList<QVariant> args = message.arguments();
    if (args.size() < 3) {
        qCWarning(lcResourceDaemonCoreLog) << "acquire: wrong argument count";
        return;
    }
    const int rtype = args[0].toInt();
    const uint clientId = args[1].toUInt();
    const uint reqno = args[2].toUInt();

           // Найти клиента по ID
    ResourceClient *client = parent()->findClientById(clientId);
    if (!client) {
        qCWarning(lcResourceDaemonCoreLog) << "acquire: client not found" << clientId;
        return;
    }

    client->setPendingReqno(reqno);
    parent()->requestResources(client, client->resources());
}

void ManagerAdaptor::handleRelease(const QDBusMessage &message, const QDBusConnection &connection)
{
    Q_UNUSED(connection);
    const QList<QVariant> args = message.arguments();
    if (args.size() < 3) {
        qCWarning(lcResourceDaemonCoreLog) << "release: wrong argument count";
        return;
    }
    int rtype = args[0].toInt();
    uint clientId = args[1].toUInt();
    uint reqno = args[2].toUInt();

    ResourceClient *client = parent()->findClientById(clientId);
    if (!client) {
        qCWarning(lcResourceDaemonCoreLog) << "release: client not found" << clientId;
        return;
    }

    parent()->releaseAll(client);
    emit parent()->clientReleased(client);

    qCDebug(lcResourceDaemonCoreLog) << "Resources released for client" << client->objectPath();
}

void ManagerAdaptor::handleUnregister(const QDBusMessage &message, const QDBusConnection &connection)
{
    Q_UNUSED(connection);
    const QList<QVariant> args = message.arguments();
    if (args.size() < 3) {
        qCWarning(lcResourceDaemonCoreLog) << "unregister: wrong argument count";
        return;
    }
    int rtype = args[0].toInt();
    uint clientId = args[1].toUInt();
    uint reqno = args[2].toUInt();

    ResourceClient *client = parent()->findClientById(clientId);
    if (!client) {
        qCWarning(lcResourceDaemonCoreLog) << "unregister: client not found" << clientId;
        return;
    }

    parent()->releaseAll(client);
    parent()->destroyClient(client);

    qCDebug(lcResourceDaemonCoreLog) << "Client unregistered:" << client->objectPath();
}

void ManagerAdaptor::handleUpdate(const QDBusMessage &message, const QDBusConnection &connection)
{
    Q_UNUSED(connection);
    const QList<QVariant> args = message.arguments();
    if (args.size() < 10) {
        qCWarning(lcResourceDaemonCoreLog) << "update: wrong argument count" << args.size();
        return;
    }
    int rtype = args[0].toInt();
    uint clientId = args[1].toUInt();
    uint reqno = args[2].toUInt();
    uint mandatory = args[3].toUInt();
    uint optional = args[4].toUInt();
    uint share = args[5].toUInt();
    uint mask = args[6].toUInt();
    QString klass = args[7].toString();
    QString mode = args[8].toString();
    uint priority = args[9].toUInt();

    ResourceClient *client = parent()->findClientById(clientId);
    if (!client) {
        qCWarning(lcResourceDaemonCoreLog) << "update: client not found" << clientId;
        return;
    }

    client->setMandatory(mandatory);
    client->setOptional(optional);
    client->setShare(share);
    client->setMask(mask);
    client->setKlass(klass);
    client->setMode(mode);
    client->setPriority(priority);

    parent()->reevaluateClient(client, reqno);

    qCDebug(lcResourceDaemonCoreLog) << "Client updated:" << client->objectPath() << "new mandatory:" << mandatory;
}

void ManagerAdaptor::handleAudio(const QDBusMessage &message, const QDBusConnection &connection)
{
    Q_UNUSED(connection);
    const QList<QVariant> args = message.arguments();
    // Ожидаемая сигнатура: (i, u, u, s, s, s, s, s)
    if (args.size() < 8) {
        qCWarning(lcResourceDaemonCoreLog) << "audio: wrong argument count";
        return;
    }
    int rtype = args[0].toInt();
    uint clientId = args[1].toUInt();
    uint reqno = args[2].toUInt();
    QString group = args[3].toString();
    QString appId = args[4].toString();
    QString property = args[5].toString();
    QString method = args[6].toString();   // match method, например "prefix", "regex"
    QString pattern = args[7].toString();

    ResourceClient *client = parent()->findClientById(clientId);
    if (!client) {
        qCWarning(lcResourceDaemonCoreLog) << "audio: client not found" << clientId;
        return;
    }

    client->setAudioSpec(group, appId, property, method, pattern);

    emit parent()->audioSpecChanged(client);

    qCDebug(lcResourceDaemonCoreLog) << "Audio spec updated for client" << client->objectPath();
}

void ManagerAdaptor::handleVideo(const QDBusMessage &message, const QDBusConnection &connection)
{
    Q_UNUSED(connection);
    const QList<QVariant> args = message.arguments();
    if (args.size() < 4) {
        qCWarning(lcResourceDaemonCoreLog) << "video: wrong argument count";
        return;
    }
    int rtype = args[0].toInt();
    uint clientId = args[1].toUInt();
    uint reqno = args[2].toUInt();
    uint pid = args[3].toUInt();

    ResourceClient *client = parent()->findClientById(clientId);
    if (!client) {
        qCWarning(lcResourceDaemonCoreLog) << "video: client not found" << clientId;
        return;
    }

    client->setVideoPid(pid);
    emit parent()->videoSpecChanged(client);

    qCDebug(lcResourceDaemonCoreLog) << "Video spec updated for client" << client->objectPath() << "pid:" << pid;
}

void ManagerAdaptor::sendGrant(ResourceClient *client, uint reqno, uint grantedMask)
{
    if (!client || client->serviceName().isEmpty())
        return;

    QDBusMessage grantCall = QDBusMessage::createMethodCall(
        client->serviceName(),
        client->objectPath(),
        "org.maemo.resource.client",
        "grant"
        );
    grantCall << (int)9 << client->clientID() << reqno << grantedMask;

    QDBusConnection::systemBus().asyncCall(grantCall);
    qCDebug(lcResourceDaemonCoreLog) << "Sent grant to" << client->objectPath() << "reqno" << reqno << "mask" << grantedMask;
}

void ManagerAdaptor::sendAdvice(ResourceClient *client, uint reqno, uint adviceMask)
{
    if (!client || client->serviceName().isEmpty())
        return;

    QDBusMessage adviceCall = QDBusMessage::createMethodCall(
        client->serviceName(),
        client->objectPath(),
        "org.maemo.resource.client",
        "advice"
        );
    adviceCall << (int)9 << client->clientID() << reqno << adviceMask;
    QDBusConnection::systemBus().asyncCall(adviceCall);
}

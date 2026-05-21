#include "tst_manageradaptor.h"
#include "core/resourcemanager.h"
#include "core/resourceclient.h"
#include "dbus/manageradaptor.h"
#include <QTest>
#include <QDBusMessage>
#include <QDBusConnection>

void tst_ManagerAdaptor::init()
{
    m_mgr = new ResourceManager();
    m_adaptor = new ManagerAdaptor(m_mgr);
}

void tst_ManagerAdaptor::cleanup()
{
    delete m_adaptor;
    delete m_mgr;
}

void tst_ManagerAdaptor::testRegisterMessage()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "test.service", "/org/maemo/resource/manager",
        "org.maemo.resource.manager", "register");
    // type, id, reqno, mandatory, optional, share, mask, class, mode, priority
    msg << 1 << 1u << 1u << 4u << 2u << 0u << 0u << "call" << "auto-release" << 50u;

    bool handled = m_adaptor->handleMessage(msg, QDBusConnection::sessionBus());
    QVERIFY(handled);
    QCOMPARE(m_mgr->clients().size(), 1);

    ResourceClient *client = m_mgr->clients().first();
    QCOMPARE(client->priority(), 50);
    QCOMPARE(client->serviceName(), QString("test.service"));
    QCOMPARE(client->mandatory(), 4u);
    QCOMPARE(client->optional(), 2u);
}

void tst_ManagerAdaptor::testAcquireMessage()
{
    // Register first
    QDBusMessage regMsg = QDBusMessage::createMethodCall(
        "test.service", "/org/maemo/resource/manager",
        "org.maemo.resource.manager", "register");
    regMsg << 1 << 1u << 1u << 4u << 2u << 0u << 0u << "call" << "auto-release" << 50u;
    m_adaptor->handleMessage(regMsg, QDBusConnection::sessionBus());

    ResourceClient *client = m_mgr->clients().first();
    uint clientId = client->clientID();

           // Acquire
    QDBusMessage acqMsg = QDBusMessage::createMethodCall(
        client->serviceName(), "/org/maemo/resource/manager",
        "org.maemo.resource.manager", "acquire");
    acqMsg << 3 << clientId << 42u;

    bool handled = m_adaptor->handleMessage(acqMsg, QDBusConnection::sessionBus());
    QVERIFY(handled);
    QCOMPARE(client->pendingReqno(), 42u);
}

void tst_ManagerAdaptor::testUnregisterMessage()
{
    // Register
    QDBusMessage regMsg = QDBusMessage::createMethodCall(
        "test.service", "/org/maemo/resource/manager",
        "org.maemo.resource.manager", "register");
    regMsg << 1 << 1u << 1u << 4u << 2u << 0u << 0u << "call" << "auto-release" << 50u;
    m_adaptor->handleMessage(regMsg, QDBusConnection::sessionBus());

    ResourceClient *client = m_mgr->clients().first();
    uint clientId = client->clientID();

           // Unregister
    QDBusMessage unregMsg = QDBusMessage::createMethodCall(
        client->serviceName(), "/org/maemo/resource/manager",
        "org.maemo.resource.manager", "unregister");
    unregMsg << 0 << clientId << 0u;

    bool handled = m_adaptor->handleMessage(unregMsg, QDBusConnection::sessionBus());
    QVERIFY(handled);
    QCOMPARE(m_mgr->clients().size(), 0);
}

void tst_ManagerAdaptor::testReleaseMessage()
{
    // Register
    QDBusMessage regMsg = QDBusMessage::createMethodCall(
        "test.service", "/org/maemo/resource/manager",
        "org.maemo.resource.manager", "register");
    regMsg << 1 << 1u << 1u << 4u << 2u << 0u << 0u << "call" << "auto-release" << 50u;
    m_adaptor->handleMessage(regMsg, QDBusConnection::sessionBus());

    ResourceClient *client = m_mgr->clients().first();
    uint clientId = client->clientID();

           // Grant some resource via manager
    m_mgr->requestResources(client, QStringList() << "AudioPlayback");
    QVERIFY(client->hasResource("AudioPlayback"));

           // Release
    QDBusMessage relMsg = QDBusMessage::createMethodCall(
        client->serviceName(), "/org/maemo/resource/manager",
        "org.maemo.resource.manager", "release");
    relMsg << 0 << clientId << 1u;

    bool handled = m_adaptor->handleMessage(relMsg, QDBusConnection::sessionBus());
    QVERIFY(handled);
    QVERIFY(!client->hasResource("AudioPlayback"));
}

void tst_ManagerAdaptor::testUpdateMessage()
{
    // Register
    QDBusMessage regMsg = QDBusMessage::createMethodCall(
        "test.service", "/org/maemo/resource/manager",
        "org.maemo.resource.manager", "register");
    regMsg << 1 << 1u << 1u << 4u << 2u << 0u << 0u << "call" << "auto-release" << 50u;
    m_adaptor->handleMessage(regMsg, QDBusConnection::sessionBus());

    ResourceClient *client = m_mgr->clients().first();
    uint clientId = client->clientID();

           // Update
    QDBusMessage updMsg = QDBusMessage::createMethodCall(
        client->serviceName(), "/org/maemo/resource/manager",
        "org.maemo.resource.manager", "update");
    updMsg << 0 << clientId << 10u << 8u << 4u << 1u << 0u << "call" << "auto-release" << 100u;

    bool handled = m_adaptor->handleMessage(updMsg, QDBusConnection::sessionBus());
    QVERIFY(handled);
    QCOMPARE(client->mandatory(), 8u);
    QCOMPARE(client->optional(), 4u);
    QCOMPARE(client->priority(), 100u);
}

QTEST_APPLESS_MAIN(tst_ManagerAdaptor)

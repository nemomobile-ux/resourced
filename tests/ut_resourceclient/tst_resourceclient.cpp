#include "tst_resourceclient.h"
#include "core/resourceclient.h"
#include <QTest>
#include <QSignalSpy>

void tst_ResourceClient::init()
{
    m_client.reset(new ResourceClient());
}

void tst_ResourceClient::cleanup()
{
    m_client.reset();
}

void tst_ResourceClient::testInitialState()
{
    QVERIFY(m_client->resources().isEmpty());
    QCOMPARE(m_client->clientID(), 0u);
    QCOMPARE(m_client->priority(), 0);
    QVERIFY(m_client->serviceName().isEmpty());
}

void tst_ResourceClient::testAddRemoveResource()
{
    m_client->addResource("AudioPlayback");
    QVERIFY(m_client->hasResource("AudioPlayback"));
    QCOMPARE(m_client->resources().size(), 1);

    m_client->removeResource("AudioPlayback");
    QVERIFY(!m_client->hasResource("AudioPlayback"));
    QVERIFY(m_client->resources().isEmpty());
}

void tst_ResourceClient::testNotifySignals()
{
    QSignalSpy spy(m_client.data(), &ResourceClient::notify);

    m_client->notifyGranted("AudioPlayback");
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), "granted");
    QCOMPARE(args.at(1).toString(), "AudioPlayback");
}

void tst_ResourceClient::testClientIDAndReqqno()
{
    m_client->setClientID(42);
    QCOMPARE(m_client->clientID(), 42u);
    QCOMPARE(m_client->clientReqqno(), 1u);
    QCOMPARE(m_client->clientReqqno(), 2u);
}

QTEST_APPLESS_MAIN(tst_ResourceClient)

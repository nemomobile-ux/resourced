#include "tst_resourcemanager.h"
#include "core/resourcemanager.h"
#include "core/resourceclient.h"
#include <QTest>
#include <QSignalSpy>

void tst_ResourceManager::init()
{
    m_mgr = new ResourceManager();
    m_clientLow = new ResourceClient();
    m_clientLow->setPriority(10);
    m_clientHigh = new ResourceClient();
    m_clientHigh->setPriority(20);
}

void tst_ResourceManager::cleanup()
{
    delete m_mgr;
    delete m_clientLow;
    delete m_clientHigh;
}

void tst_ResourceManager::testSingleClientAcquire()
{
    m_mgr->requestResources(m_clientLow, QStringList() << "AudioPlayback");
    QVERIFY(m_clientLow->hasResource("AudioPlayback"));
    QVERIFY(m_mgr->isOwner("AudioPlayback", m_clientLow));
}

void tst_ResourceManager::testPreemptHigherPriority()
{
    m_mgr->requestResources(m_clientLow, QStringList() << "AudioPlayback");
    QVERIFY(m_clientLow->hasResource("AudioPlayback"));

    m_mgr->requestResources(m_clientHigh, QStringList() << "AudioPlayback");
    QVERIFY(!m_clientLow->hasResource("AudioPlayback"));
    QVERIFY(m_clientHigh->hasResource("AudioPlayback"));
    QVERIFY(m_mgr->isOwner("AudioPlayback", m_clientHigh));
}

void tst_ResourceManager::testNoPreemptSamePriority()
{
    ResourceClient clientA, clientB;
    clientA.setPriority(15);
    clientB.setPriority(15);

    m_mgr->requestResources(&clientA, QStringList() << "AudioPlayback");
    QVERIFY(clientA.hasResource("AudioPlayback"));

    m_mgr->requestResources(&clientB, QStringList() << "AudioPlayback");
    QVERIFY(clientA.hasResource("AudioPlayback"));
    QVERIFY(!clientB.hasResource("AudioPlayback"));
}

void tst_ResourceManager::testReleaseResources()
{
    m_mgr->requestResources(m_clientLow, QStringList() << "AudioPlayback");
    QVERIFY(m_clientLow->hasResource("AudioPlayback"));

    m_mgr->releaseAll(m_clientLow);
    QVERIFY(!m_clientLow->hasResource("AudioPlayback"));
    QVERIFY(!m_mgr->isOwner("AudioPlayback", m_clientLow));
}

void tst_ResourceManager::testReevaluateClient()
{
    // Assuming mask: AudioPlayback=1, VideoPlayback=2
    m_clientLow->setMandatory(1);
    m_clientLow->setOptional(2);
    m_mgr->requestResources(m_clientLow, QStringList() << "AudioPlayback");
    QVERIFY(m_clientLow->hasResource("AudioPlayback"));
    QVERIFY(!m_clientLow->hasResource("VideoPlayback"));

    m_mgr->reevaluateClient(m_clientLow, 100);
    QVERIFY(m_clientLow->hasResource("VideoPlayback"));
    QCOMPARE(m_clientLow->granted(), 3u);
}

void tst_ResourceManager::testGrantSignal()
{
    QSignalSpy grantSpy(m_mgr, &ResourceManager::grantResource);
    QSignalSpy adviceSpy(m_mgr, &ResourceManager::adviceResource);

    m_mgr->requestResources(m_clientLow, QStringList() << "AudioPlayback");
    QTest::qWait(1); // allow async processing

    QCOMPARE(grantSpy.count(), 1);
    QList<QVariant> args = grantSpy.takeFirst();
    QCOMPARE(qvariant_cast<ResourceClient*>(args[0]), m_clientLow);
    QCOMPARE(args[2].toUInt(), 1u); // mask
}

QTEST_APPLESS_MAIN(tst_ResourceManager)

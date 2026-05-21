#ifndef TST_RESOURCEMANAGER_H
#define TST_RESOURCEMANAGER_H

#include <QObject>

class ResourceManager;
class ResourceClient;

class tst_ResourceManager : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void testSingleClientAcquire();
    void testPreemptHigherPriority();
    void testNoPreemptSamePriority();
    void testReleaseResources();
    void testReevaluateClient();
    void testGrantSignal();

private:
    ResourceManager *m_mgr;
    ResourceClient *m_clientLow;
    ResourceClient *m_clientHigh;
};

#endif

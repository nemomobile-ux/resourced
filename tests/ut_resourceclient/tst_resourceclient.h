#ifndef TST_RESOURCECLIENT_H
#define TST_RESOURCECLIENT_H

#include <QObject>
#include <QScopedPointer>
#include "core/resourceclient.h"

class tst_ResourceClient : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void testInitialState();
    void testAddRemoveResource();
    void testNotifySignals();
    void testClientIDAndReqqno();

private:
    QScopedPointer<ResourceClient> m_client;
};

#endif

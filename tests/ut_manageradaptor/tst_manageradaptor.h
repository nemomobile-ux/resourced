#ifndef TST_MANAGERADAPTOR_H
#define TST_MANAGERADAPTOR_H

#include <QObject>

class ResourceManager;
class ManagerAdaptor;

class tst_ManagerAdaptor : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void testRegisterMessage();
    void testAcquireMessage();
    void testUnregisterMessage();
    void testReleaseMessage();
    void testUpdateMessage();

private:
    ResourceManager *m_mgr;
    ManagerAdaptor *m_adaptor;
};

#endif

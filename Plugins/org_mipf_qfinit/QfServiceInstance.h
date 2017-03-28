#include <QObject>
#include "QfService.h"
class QfServiceInstance :public QObject,public QfService
{
    Q_OBJECT
    Q_INTERFACES(QfService)
public:
    QfServiceInstance();
    virtual bool Init(QApplication* app);
    virtual const char* GetServiceID() { return "Qf Service"; }

};
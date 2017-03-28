#ifndef QfService_h__
#define QfService_h__

#include <QObject>
class QfService
{
public:
    virtual bool Init(QApplication* app) = 0;
    virtual const char* GetServiceID() = 0;
};

Q_DECLARE_INTERFACE(QfService, "QfService");

#endif // QfService_h__
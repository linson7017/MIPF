#ifndef ServiceOne_h__
#define ServiceOne_h__

#include "IService.h"

#include "QObject"

class ServiceOne :public QObject, public IService
{
    Q_OBJECT
    Q_INTERFACES(IService)
public:
    ServiceOne() {}
    ~ServiceOne() {}

    void SetParameter(int x) { m_x = x; }
    int GetParameter() { return m_x; }

private:
    int m_x;
};
#endif // ServiceOne_h__
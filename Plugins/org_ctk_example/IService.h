#include <QObject>

class IService
{
public:
    virtual void SetParameter(int x) = 0;
    virtual int GetParameter() = 0;
};

Q_DECLARE_INTERFACE(IService, "Service")


#ifndef BaseApp_h__
#define BaseApp_h__

#include <QtPlugin>  
class BaseApp
{
public:
    virtual void Init() = 0;
};

Q_DECLARE_INTERFACE(BaseApp, "BaseApp/1.0");

#endif // BaseApp_h__
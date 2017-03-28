#include "BaseApp.h"
#include <iostream>

class BaseAppInstance :public QObject, public BaseApp
{
    Q_OBJECT
    //Q_PLUGIN_METADATA(IID BaseAppInstanceID)

    Q_INTERFACES(BaseApp)

#if (QT_VERSION > QT_VERSION_CHECK(5,0,0))
    Q_PLUGIN_METADATA(IID "BaseAppInstance_ID")
#endif
public:
    BaseAppInstance() {}
    virtual void Init();

};
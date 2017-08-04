#ifndef IQF_MitkDisplayOption_h__
#define IQF_MitkDisplayOption_h__

#pragma once

const char QF_MitkMain_DisplayOption[] = "QF_MitkMain_DisplayOption";

namespace mitk
{
    class DataNode;
}

class IQF_MitkDisplayOption
{
public:
    virtual void SetLevelWindow(mitk::DataNode* pNode, double dLevel, double dWindow) = 0;
};

#endif // IQF_MitkDisplayOption_h__

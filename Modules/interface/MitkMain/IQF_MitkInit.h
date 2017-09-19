#ifndef IQF_MitkInit_h__
#define IQF_MitkInit_h__


const char QF_MitkMain_Init[] = "QF_MitkMain_Init";

namespace mitk
{
    class DataStorage;
}

class IQF_MitkInit
{
public:
    virtual void Init(mitk::DataStorage* dataStorage = nullptr) = 0;
};


#endif // IQF_MitkInit_h__

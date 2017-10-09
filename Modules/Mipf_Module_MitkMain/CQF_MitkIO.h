#ifndef CQF_MitkIO_h__
#define CQF_MitkIO_h__

#pragma once
#include "MitkMain/IQF_MitkIO.h"
#include "mitkBaseData.h"

namespace QF
{
    class IQF_Main;
}


class CQF_MitkIO :public IQF_MitkIO
{
public:
    CQF_MitkIO(QF::IQF_Main* pMain);
    ~CQF_MitkIO();
    virtual mitk::DataNode* Load(const char* szFilename);
    virtual void LoadFiles();
    virtual void OpenProject();
    virtual void SaveProject();
    virtual void Save(const mitk::BaseData *data);
    virtual void Save(const std::vector<const mitk::BaseData *> &data);
private:
    QF::IQF_Main* m_pMain;
};

#endif // CQF_MitkIO_h__

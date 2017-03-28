#include "CQF_MitkReference.h"

#include <mitkIOUtil.h>
#include "mitkRenderingManager.h"
#include <QSettings>

CQF_MitkReference::CQF_MitkReference()
{
    m_Settings = new QSettings("config.ini", QSettings::IniFormat);
}


CQF_MitkReference::~CQF_MitkReference()
{
}


const char* CQF_MitkReference::GetStringConfig(const char* szID)
{
    m_Settings->beginGroup("config");
    std::string value =  m_Settings->value(szID).toString().toStdString();
    m_Settings->endGroup();
    return value.c_str();
}

void CQF_MitkReference::SetStringConfig(const char* szID, const char* szValue)
{
    m_Settings->beginGroup("config");
    m_Settings->setValue(szID, szValue);
    m_Settings->endGroup();  
}
#include "CQF_MitkReference.h"

#include <mitkIOUtil.h>
#include "mitkRenderingManager.h"
#include <QSettings>

#include <algorithm>

#include <QStringList>

CQF_MitkReference::CQF_MitkReference()
{
    m_Settings = new QSettings("config.ini", QSettings::IniFormat);
}


CQF_MitkReference::~CQF_MitkReference()
{
}


const char* CQF_MitkReference::GetString(const char* szID, const char* szDef)
{
    m_Settings->beginGroup("config");
    std::string value =  m_Settings->value(szID, szDef).toString().toStdString();
    m_Settings->endGroup();
    return value.c_str();
}

void CQF_MitkReference::SetString(const char* szID, const char* szValue)
{
    m_Settings->beginGroup("config");
    m_Settings->setValue(szID, szValue);
    m_Settings->endGroup();  
}

bool CQF_MitkReference::GetBool(const char* szID, bool bDef)
{
	m_Settings->beginGroup("config");
	bool value = m_Settings->value(szID, bDef).toBool();
	m_Settings->endGroup();
	return value;
}
void CQF_MitkReference::SetBool(const char* szID, bool value)
{
	m_Settings->beginGroup("config");
	m_Settings->setValue(szID, value);
	m_Settings->endGroup();
}
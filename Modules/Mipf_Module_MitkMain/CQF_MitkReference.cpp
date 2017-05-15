#include "CQF_MitkReference.h"

#include <mitkIOUtil.h>
#include "mitkRenderingManager.h"
#include <QSettings>

#include <algorithm>
#include "iqf_main.h"

#include <QStringList>

CQF_MitkReference::CQF_MitkReference(QF::IQF_Main* pMain)
{
    m_pMain = pMain;
    std::string configPath = m_pMain->GetConfigPath();
    m_Settings = new QSettings(configPath.append("/config.ini").c_str(), QSettings::IniFormat);
}


CQF_MitkReference::~CQF_MitkReference()
{
    delete m_Settings;
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

int CQF_MitkReference::GetInt(const char* szID, int bDef)
{
    m_Settings->beginGroup("config");
    int value = m_Settings->value(szID, bDef).toInt();
    m_Settings->endGroup();
    return value;
}

void CQF_MitkReference::SetInt(const char* szID, int value)
{
    m_Settings->beginGroup("config");
    m_Settings->setValue(szID, value);
    m_Settings->endGroup();
}
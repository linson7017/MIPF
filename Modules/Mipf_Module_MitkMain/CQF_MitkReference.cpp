#include "CQF_MitkReference.h"

#include <mitkIOUtil.h>
#include "mitkRenderingManager.h"
#include "MitkMain/mitk_main_msg.h"


#include <algorithm>
#include "iqf_main.h"

#include <QStringList>
#include <QDebug>
#include <QSettings>

CQF_MitkReference::CQF_MitkReference(QF::IQF_Main* pMain)
{
    m_pMain = pMain;
    std::string configPath = m_pMain->GetConfigPath();
    m_Settings = new QSettings(QString::fromLocal8Bit(configPath.c_str()).append("/config.ini"), QSettings::IniFormat);
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
    m_pMain->SendMessageQf(MITK_MESSAGE_REFERENCE_STRING_CHANGED, 0, &std::make_pair(szID,szValue));
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
    m_pMain->SendMessageQf(MITK_MESSAGE_REFERENCE_BOOL_CHANGED, 0, &std::make_pair(szID, value));

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
    m_pMain->SendMessageQf(MITK_MESSAGE_REFERENCE_INT_CHANGED, 0, &std::make_pair(szID, value));

}

double CQF_MitkReference::GetDouble(const char* szID, double bDef )
{
    m_Settings->beginGroup("config");
    double value = m_Settings->value(szID, bDef).toDouble();
    m_Settings->endGroup();
    return value;
}

void CQF_MitkReference::SetDouble(const char* szID, double value)
{
    m_Settings->beginGroup("config");
    m_Settings->setValue(szID, value);
    m_Settings->endGroup();
    m_pMain->SendMessageQf(MITK_MESSAGE_REFERENCE_DOUBLE_CHANGED, 0, &std::make_pair(szID, value));

}
#ifndef template_h__
#define template_h__


const char CMakeListTemplate[] = ""\
"cmake_minimum_required(VERSION 2.8.11)\n"\
"if (POLICY CMP0020)\n"\
"   cmake_policy(SET CMP0020 NEW)\n"\
"endif()\n"\
"if (POLICY CMP0025)\n"\
"   cmake_policy(SET CMP0025 NEW) # CMake 3.0\n"\
"endif()\n"\
"if (POLICY CMP0043)\n"\
"   cmake_policy(SET CMP0043 NEW) # CMake 3.0\n"\
"endif()\n"\
"if (POLICY CMP0053)\n"\
"   cmake_policy(SET CMP0053 NEW) # CMake 3.1\n"\
"endif()\n"\
"\n"\
"project(@ProjectName@)\n"\
"\n"\
"file(GLOB Headers \"./*.h\")\n"\
"AUX_SOURCE_DIRECTORY(./ DIR_SRCS)\n"\
"\n"\
"FIND_PACKAGE(PocoFoundation REQUIRED)\n"\
"\n"\
"add_library(@ProjectName@ SHARED ${DIR_SRCS} ${Headers})\n"\
"target_link_libraries(@ProjectName@ @VTK_LIBRARIES@ @ITK_LIBRARIES@ @QTFRAMEWORK_LIBRARIES@ @QFMAIN_LIBRARIES@ @MitkQtWidgets@ @MitkQtWidgetsExt@)\n"\
"\n";

const char ComH[] = ""\
"#ifndef @ComName@_h__\n"\
"#define @ComName@_h__\n"\
"\n"\
"#pragma once\n"\
"#include \"iqf_component.h\"\n"\
"\n"\
"class @CommandName@;\n"\
"class @MessageName@;\n"\
"\n"\
"class @ComName@ :public QF::IQF_Component\n"\
"{\n"\
"public:\n"\
"    @ComName@(QF::IQF_Main* pMain);\n"\
"    ~@ComName@();\n"\
"    virtual void Release();\n"\
"    virtual bool Init();\n"\
"    virtual void* GetInterfacePtr(const char* szInterfaceID);\n"\
"    const char* GetComponentID() { return \"QF_Component_@ComName@\"; }\n"\
"    int GetInterfaceCount();\n"\
"    const char* GetInterfaceID(int iID);\n"\
"private:\n"\
"    @CommandName@* m_pMainCommand;\n"\
"    @MessageName@* m_pMainMessage;\n"\
"\n"\
"   QF::IQF_Main* m_pMain;\n"\
"};\n"\
"\n"\
"#endif // @ComName@_h__";


const char ComC[] = ""\
"#include \"@ComName@.h\"\n"\
"#include <string>\n"\
"#include <assert.h>\n"\
"\n"\
"#include \"@CommandName@.h\"\n"\
"#include \"@MessageName@.h\"\n"\
"#include \"internal/qf_interfacedef.h\"\n"\
"\n"\
"QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)\n"\
"{\n"\
"    QF::IQF_Component* pComponent = new @ComName@(pMain);\n"\
"    assert(pComponent);\n"\
"    return pComponent;\n"\
"}\n"\
"\n"\
"@ComName@::@ComName@(QF::IQF_Main* pMain) :m_pMain(pMain)\n"\
"{\n"\
"\n"\
"}\n"\
"\n"\
"@ComName@::~@ComName@()\n"\
"{\n"\
"    m_pMainCommand->Release();\n"\
"    m_pMainMessage->Release();\n"\
"}\n"\
"\n"\
"void @ComName@::Release()\n"\
"{\n"\
"    delete this;\n"\
"}\n"\
"\n"\
"bool @ComName@::Init()\n"\
"{\n"\
"    m_pMainCommand = new @CommandName@(m_pMain);\n"\
"    m_pMainMessage = new @MessageName@(m_pMain);\n"\
"    return true;\n"\
"}\n"\
"\n"\
"int @ComName@::GetInterfaceCount()\n"\
"{\n"\
"    return 2;\n"\
"}\n"\
"\n"\
"const char* @ComName@::GetInterfaceID(int iID)\n"\
"{\n"\
"    switch (iID)\n"\
"    {\n"\
"    case 0:\n"\
"        return QF_INTERFACE_MAIN_COMMAND;\n"\
"    case 1:\n"\
"        return QF_INTERFACE_MAIN_MESSAGE;\n"\
"    default:\n"\
"        break;\n"\
"    }\n"\
"    return \"\";\n"\
"}\n"\
"\n"\
"void* @ComName@::GetInterfacePtr(const char* szInterfaceID)\n"\
"{\n"\
"    if (strcmp(szInterfaceID, QF_INTERFACE_MAIN_COMMAND) == 0)\n"\
"    {\n"\
"        return m_pMainCommand;\n"\
"    }\n"\
"    else if (strcmp(szInterfaceID, QF_INTERFACE_MAIN_MESSAGE) == 0)\n"\
"    {\n"\
"        return m_pMainMessage;\n"\
"    }\n"\
"    else\n"\
"        return NULL;\n"\
"}";

const char CommandH[] = ""\
"#ifndef @CommandName@_h__\n"\
"#define @CommandName@_h__\n"\
"#include \"qf_config.h\"\n"\
"#include \"iqf_command.h\"\n"\
"\n"\
"#pragma once\n"\
"\n"\
"namespace QF {\n"\
"    class IQF_Main;\n"\
"}\n"\
"\n"\
"class @CommandName@ :public QF::IQF_Command\n"\
"{\n"\
"public:\n"\
"    @CommandName@(QF::IQF_Main* pMain);\n"\
"    ~@CommandName@();\n"\
"    void Release();\n"\
"    virtual bool ExecuteCommand(const char* szCommandID, QF::IQF_PropertySet* pInParam, QF::IQF_PropertySet* pOutParam);\n"\
"    virtual int GetCommandCount();\n"\
"    virtual const char* GetCommandID(int iIndex);\n"\
"private:\n"\
"\n"\
"    QF::IQF_Main* m_pMain;\n"\
"};\n"\
"\n"\
"#endif // @CommandName@_h__"; 


const char CommandC[] = ""\
"#include \"@CommandName@.h\"\n"\
"#include <string.h>\n"\
"#include \"iqf_main.h\"\n"\
"\n"\
"@CommandName@::@CommandName@(QF::IQF_Main* pMain)\n"\
"{\n"\
"    m_pMain = pMain;\n"\
"}\n"\
"\n"\
"@CommandName@::~@CommandName@()\n"\
"{\n"\
"}\n"\
"\n"\
"void @CommandName@::Release()\n"\
"{\n"\
"   delete this;\n"\
"}\n"\
"\n"\
"bool @CommandName@::ExecuteCommand(const char* szCommandID, QF::IQF_PropertySet* pInParam, QF::IQF_PropertySet* pOutParam)\n"\
"{\n"\
"    if (strcmp(szCommandID, \"\") == 0)\n"\
"    {      \n"\
"        return true;\n"\
"    }\n"\
"    else\n"\
"    {\n"\
"        return false;\n"\
"    }\n"\
"}\n"\
"\n"\
"int @CommandName@::GetCommandCount()\n"\
"{\n"\
"    return 1;\n"\
"}\n"\
"\n"\
"const char* @CommandName@::GetCommandID(int iIndex)\n"\
"{\n"\
"    switch (iIndex)\n"\
"    {\n"\
"    case 0:\n"\
"        return \"\";\n"\
"    default:\n"\
"        return \"\";\n"\
"        break;\n"\
"    }\n"\
"}\n"\
"\n";

const char MessageH[] = ""\
"#ifndef @MessageName@_h__\n"\
"#define @MessageName@_h__\n"\
"#include \"qf_config.h\"\n"\
"#include \"iqf_message.h\"\n"\
"\n"\
"namespace QF\n"\
"{\n"\
"    class IQF_Main;\n"\
"}\n"\
"\n"\
"class @MessageName@ : public QF::IQF_Message\n"\
"{\n"\
"public:\n"\
"    @MessageName@(QF::IQF_Main* pMain);\n"\
"    ~@MessageName@();\n"\
"    void Release();\n"\
"    virtual int GetMessageCount();\n"\
"    virtual const char* GetMessageID(int iIndex);\n"\
"    virtual void OnMessage(const char* szMessage, int iValue, void *pValue);\n"\
"private:\n"\
"    QF::IQF_Main*  m_pMain;\n"\
"};\n"\
"\n"\
"#endif // @MessageName@_h__\n";

const char MessageC[] = ""\
"#include \"@MessageName@.h\"\n"\
"#include \"MitkMain/mitk_main_msg.h\"\n"\
"#include <string.h>\n"\
"#include \"iqf_main.h\"\n"\
"\n"\
"@MessageName@::@MessageName@(QF::IQF_Main* pMain) :m_pMain(pMain)\n"\
"{\n"\
"\n"\
"}\n"\
"\n"\
"@MessageName@::~@MessageName@()\n"\
"{\n"\
"\n"\
"}\n"\
"\n"\
"void @MessageName@::Release()\n"\
"{\n"\
"    delete this;\n"\
"}\n"\
"\n"\
"int @MessageName@::GetMessageCount()\n"\
"{\n"\
"    return 1;\n"\
"}\n"\
"\n"\
"const char* @MessageName@::GetMessageID(int iIndex)\n"\
"{\n"\
"    switch (iIndex)\n"\
"    {\n"\
"    case 0:\n"\
"        return \"\";\n"\
"    }\n"\
"    return \"\";\n"\
"}\n"\
"\n"\
"void @MessageName@::OnMessage(const char* szMessage, int iValue, void *pValue)\n"\
"{\n"\
"    if (strcmp(szMessage, \"\") == 0)\n"\
"    {\n"\
"    }\n"\
"}";
#endif // template_h__

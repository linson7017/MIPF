#ifndef template_h__
#define template_h__


const char CMakeListTemplate[] = ""\
"cmake_minimum_required(VERSION 2.8.11)\n"\
"if (POLICY CMP0020)\n"\
"cmake_policy(SET CMP0020 NEW)\n"\
"endif()\n"\
"if (POLICY CMP0025)\n"\
"cmake_policy(SET CMP0025 NEW) # CMake 3.0\n"\
"endif()\n"\
"if (POLICY CMP0043)\n"\
"cmake_policy(SET CMP0043 NEW) # CMake 3.0\n"\
"endif()\n"\
"if (POLICY CMP0053)\n"\
"cmake_policy(SET CMP0053 NEW) # CMake 3.1\n"\
"endif()\n"\
"\n"\
"project(@PluginName@)\n"\
""\
"file(GLOB Headers \"./*.h\")\n"\
"AUX_SOURCE_DIRECTORY(. DIR_SRCS)\n"\
"\n"\
"set(CMAKE_AUTOMOC ON)\n"\
"\n"\
"if (VTK_QT_VERSION VERSION_GREATER \"4\")\n"\
"\n"\
"    add_library(@PluginName@ SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})\n"\
"    qt5_use_modules(@PluginName@ Core Gui Widgets)\n"\
"    target_link_libraries(@PluginName@ @VTK_LIBRARIES@ @ITK_LIBRARIES@ @QTFRAMEWORK_LIBRARIES@ @QFMAIN_LIBRARIES@ @MitkQtWidgets@ @MitkQtWidgetsExt@)\n"\
"\n"\
"else()\n"\
""\
"    add_library(@PluginName@ SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})\n"\
"    target_link_libraries(@PluginName@ ${QT_LIBRARIES@} @VTK_LIBRARIES@ @ITK_LIBRARIES@ @QTFRAMEWORK_LIBRARIES@ @QFMAIN_LIBRARIES@ @MitkQtWidgets@ @MitkQtWidgetsExt@)\n"\
"\n"\
"endif()\n";

const char ViewH[] = ""\
"#ifndef @ViewName@_h__\n"\
"#define @ViewName@_h__\n"\
"\n"\
"#include \"PluginView.h\"\n"\
"\n"\
"class @ViewName@ : public PluginView\n"\
"{\n"\
"public:\n"\
"    @ViewName@(QF::IQF_Main* pMain);\n"\
"protected:\n"\
"    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);\n"\
"};\n"\
"\n"\
"#endif // @ViewName@_h__";


const char ViewC[] = ""\
"#include \"@ViewName@.h\"\n"\
"#include \"iqf_main.h\"\n"\
"#include \"Res/R.h\"\n"\
"@ViewName@::@ViewName@(QF::IQF_Main* pMain) :PluginView(pMain)\n"\
"{\n"\
"    m_pMain->Attach(this);\n"\
"}\n"\
"\n"\
"void @ViewName@::Update(const char* szMessage, int iValue, void* pValue)\n"\
"{\n"\
"    if (strcmp(szMessage, \"\") == 0)\n"\
"    {\n"\
"        //do what you want for the message\n"\
"    }\n"\
"}";

const char ViewActivatorH[] = ""\
"#ifndef @PluginName@Activator_h__\n"\
"#define @PluginName@Activator_h__\n"\
"\n"\
"#pragma once\n"\
"#include \"iqf_activator.h\"\n"\
"\n"\
"class @ViewName@;\n"\
"\n"\
"class @PluginName@_Activator : public QF::IQF_Activator\n"\
"{\n"\
"public:\n"\
"    @PluginName@_Activator(QF::IQF_Main* pMain);\n"\
"\n"\
"    bool Init();\n"\
"    const char* GetID();\n"\
"    void Register(R* pR);\n"\
"private:\n"\
"    @ViewName@* m_p@ViewName@;\n"\
"    QF::IQF_Main* m_pMain;\n"\
"};\n"\
"\n"\
"#endif // @PluginName@Activator_h__"; 


const char ViewActivatorC[] = ""\
"#include \"@PluginName@Activator.h\"\n"\
"#include \"@ViewName@.h\"\n"\
"\n"\
"QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)\n"\
"{\n"\
"    QF::IQF_Activator* pActivator = new @PluginName@_Activator(pMain);\n"\
"    //assert(pActivator);\n"\
"    return pActivator;\n"\
"}\n"\
"\n"\
"const char @PluginName@_Activator_Activator_ID[] = \"@PluginName@_Activator_Activator_ID\";\n"\
"\n"\
"@PluginName@_Activator::@PluginName@_Activator(QF::IQF_Main* pMain)\n"\
"{\n"\
"   m_pMain = pMain; \n"\
"}\n"\
"\n"\
"bool @PluginName@_Activator::Init()\n"\
"{\n"\
"    m_p@ViewName@ = new @ViewName@(m_PMain); \n"\
"    return true; \n"\
"}\n"\
"\n"\
"const char* @PluginName@_Activator::GetID()\n"\
"{\n"\
"    return @PluginName@_Activator_Activator_ID; \n"\
"}\n"\
"\n"\
"void @PluginName@_Activator::Register(R* pR)\n"\
"{\n"\
"    m_p@ViewName@->InitResource(pR); \n"\
"}";

#endif // template_h__

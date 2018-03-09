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
"file(GLOB UI_Srcs \"./*.ui\")\n"\
"AUX_SOURCE_DIRECTORY(. DIR_SRCS)\n"\
"\n"\
"set(CMAKE_AUTOMOC ON)\n"\
"\n"\
"qt5_wrap_ui(ui_Files ${UI_Srcs})\n"\
"add_library(@PluginName@ SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})\n"\
"qt5_use_modules(@PluginName@ Core Gui Widgets)\n"\
"target_link_libraries(@PluginName@    \n"\
"@VTK_LIBRARIES@ @ITK_LIBRARIES@  @VMTK_LIBRARIES@ \n"\
"@MitkQtWidgets@ @MitkQtWidgetsExt@)\n"\
"link_framework()\n"\
"\n";

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
"    ~@ViewName@();\n"\
"protected:\n"\
"    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);\n"\
"};\n"\
"\n"\
"#endif // @ViewName@_h__";

const char MitkPluginH[] = ""\
"#ifndef @ViewName@_h__ \n"\
"#define @ViewName@_h__ \n"\
" \n"\
"#include \"MitkPluginView.h\" \n"\
" \n"\
"class @ViewName@ : public MitkPluginView  \n"\
"{  \n"\
"public:   \n"\
"    @ViewName@(); \n"\
"    ~@ViewName@();\n"\
"    void CreateView() override;\n"\
"    WndHandle GetPluginHandle() override;\n"\
"};\n"\
"#endif // @ViewName@_h__ ";


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

const char MitkPluginC[] = ""\
"#include \"@ViewName@.h\" \n"\
"#include \"iqf_main.h\"  \n"\
"  \n"\
"@ViewName@::@ViewName@() :MitkPluginView() \n"\
"{\n"\
"}\n"\
" \n"\
"@ViewName@::~@ViewName@() \n"\
"{\n"\
"}\n"\
" \n"\
"void @ViewName@::CreateView()\n"\
"{\n"\
" \n"\
"} \n"\
" \n"\
"WndHandle @ViewName@::GetPluginHandle() \n"\
"{\n"\
"    return nullptr; \n"\
"}";

const char ViewActivatorH[] = ""\
"#ifndef @PluginName@Activator_h__\n"\
"#define @PluginName@Activator_h__\n"\
"\n"\
"#pragma once\n"\
"#include \"Activator_Base.h\"\n"\
"\n"\
"class @ViewName@;\n"\
"\n"\
"class @PluginName@_Activator : public ActivatorBase\n"\
"{\n"\
"public:\n"\
"    @PluginName@_Activator(QF::IQF_Main* pMain);\n"\
"\n"\
"    bool Init();\n"\
"    const char* GetID();\n"\
"    void Register();\n"\
"private:\n"\
"    @ViewName@* m_p@ViewName@;\n"\
"};\n"\
"\n"\
"#endif // @PluginName@Activator_h__"; 

const char  MitkViewActivatorH[] = ""\
"#ifndef @PluginName@Activator_h__\n"\
"#define @PluginName@Activator_h__\n"\
"\n"\
"#pragma once\n"\
"#include \"Activator_Base.h\"\n"\
"\n"\
"class @PluginName@_Activator : public ActivatorBase\n"\
"{\n"\
"public:\n"\
"    @PluginName@_Activator(QF::IQF_Main* pMain);\n"\
"\n"\
"    bool Init();\n"\
"    const char* GetID();\n"\
"    void Register();\n"\
"};\n"\
"\n"\
"#endif // @PluginName@Activator_h__";


const char ViewActivatorC[] = ""\
"#include \"@PluginName@Activator.h\"\n"\
"#include \"@ViewName@.h\"\n"\
"#include \"Res/R.h\"\n"\
"\n"\
"QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)\n"\
"{\n"\
"    QF::IQF_Activator* pActivator = new @PluginName@_Activator(pMain);\n"\
"    //assert(pActivator);\n"\
"    return pActivator;\n"\
"}\n"\
"\n"\
"const char @PluginName@_Activator_ID[] = \"@PluginName@_Activator_ID\";\n"\
"\n"\
"@PluginName@_Activator::@PluginName@_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)\n"\
"{\n"\
"   \n"\
"}\n"\
"\n"\
"bool @PluginName@_Activator::Init()\n"\
"{\n"\
"    m_p@ViewName@ = new @ViewName@(m_pMain); \n"\
"    return true; \n"\
"}\n"\
"\n"\
"const char* @PluginName@_Activator::GetID()\n"\
"{\n"\
"    return @PluginName@_Activator_ID; \n"\
"}\n"\
"\n"\
"void @PluginName@_Activator::Register()\n"\
"{\n"\
"    m_p@ViewName@->InitResource(); \n"\
"   // pR->registerCustomWidget(\"@ViewName@\", m_p@ViewName@); \n"\
"}";

const char MitkViewActivatorC[] = ""\
"#include \"@PluginName@Activator.h\"\n"\
"#include \"@ViewName@.h\" \n"\
"#include \"Utils/PluginFactory.h\" \n"\
"\n"\
"QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)\n"\
"{\n"\
"    QF::IQF_Activator* pActivator = new @PluginName@_Activator(pMain);\n"\
"    //assert(pActivator);\n"\
"    return pActivator;\n"\
"}\n"\
"\n"\
"const char @PluginName@_Activator_ID[] = \"@PluginName@_Activator_ID\";\n"\
"\n"\
"@PluginName@_Activator::@PluginName@_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)\n"\
"{\n"\
"   \n"\
"}\n"\
"\n"\
"bool @PluginName@_Activator::Init()\n"\
"{\n"\
"    return true; \n"\
"}\n"\
"\n"\
"const char* @PluginName@_Activator::GetID()\n"\
"{\n"\
"    return @PluginName@_Activator_ID; \n"\
"}\n"\
"\n"\
"void @PluginName@_Activator::Register()\n"\
"{\n"\
"    REGISTER_PLUGIN(\"@ViewName@\", @ViewName@);\n"\
"}";


const char UIForm[] = "" \
"<?xml version =\"1.0\" encoding =\"UTF-8\"?>\n"\
"<ui version=\"4.0\">\n"\
"<author></author> \n"\
"<comment></comment> \n"\
"<exportmacro></exportmacro> \n"\
"<class>@ViewName@</class> \n"\
"<widget class=\"QWidget\" name=\"@ViewName@\"> \n"\
"<property name=\"objectName\"> \n"\
"<string notr=\"true\">Form</string>\n"\
"</property>\n"\
"<property name=\"geometry\">\n"\
"<rect>\n"\
"<x>0</x>\n"\
"<y>0</y>\n"\
"<width>400</width>\n"\
"<height>300</height>\n"\
"</rect>\n"\
"</property>\n"\
"<property name=\"windowTitle\">\n"\
"<string>Form</string>\n"\
"</property>\n"\
"</widget>\n"\
"<pixmapfunction></pixmapfunction>\n"\
"<connections/>\n"\
"</ui>";

const char MitkUIForm[] = ""\
"<?xml version =\"1.0\" encoding =\"UTF-8\"?>\n"\
"<ui version =\"4.0\">\n"\
"<class>@ViewName@</class>\n"\
"<widget class =\"QWidget\" name =\"@ViewName@\">  \n"\
"<property name =\"geometry\">   \n"\
"<rect>  \n"\
"<x>0</x>\n"\
"<y>0</y> \n"\
"<width>400</width> \n"\
"<height>300</height>\n"\
"</rect> \n"\
"</property> \n"\
"<property name =\"windowTitle\">\n"\
"<string>Form</string> \n"\
"</property> \n"\
"<widget class =\"QWidget\" name =\"verticalLayoutWidget\"> \n"\
"<property name =\"geometry\"> \n"\
"<rect> \n"\
"<x>10</x> \n"\
"<y>10</y> \n"\
"<width>381</width>\n"\
"<height>53</height>  \n"\
"</rect> \n"\
"</property>\n"\
"<layout class =\"QVBoxLayout\" name =\"verticalLayout\">  \n"\
"<item>\n"\
"<layout class =\"QHBoxLayout\" name =\"horizontalLayout\">\n"\
"<item>\n"\
"<widget class =\"QLabel\" name =\"label\"> \n"\
"<property name =\"text\"> \n"\
"<string>Data</string>\n"\
"</property>  \n"\
"</widget> \n"\
"</item>   \n"\
"<item>  \n"\
"<widget class =\"QmitkDataStorageComboBox\" name =\"DataSelector\"/>\n"\
"</item>   \n"\
"</layout>  \n"\
"</item>   \n"\
"<item>  \n"\
"<widget class =\"QPushButton\" name =\"ApplyBtn\">  \n"\
"<property name =\"text\">  \n"\
"<string>Apply</string> \n"\
"</property> \n"\
"</widget> \n"\
"</item>  \n"\
"</layout>  \n"\
"</widget>  \n"\
"</widget>  \n"\
"<customwidgets> \n"\
"<customwidget>  \n"\
"<class>QmitkDataStorageComboBox</class>   \n"\
"<extends>QComboBox</extends>\n"\
"<header>QmitkDataStorageComboBox.h</header> \n"\
"</customwidget>  \n"\
"</customwidgets> \n"\
"<resources/> \n"\
"<connections/>   \n"\
"</ui>  ";



#endif // template_h__

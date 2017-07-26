#include "MainWindow.h"
#include <iostream>
//qt
#include <QtWidgets>
#include <QtCore>
#include <QtGui>

//qf
#include "Res/R.h"
#include "Common/app_env.h"
#include "Utils/variant.h"

//qfmain
#include "iqf_main.h"


#include "template.h"

MainWindow::MainWindow(const char* xmlfile):
m_CMakeListText(""),
m_PluginName(""),
m_ViewName(""),
m_PluginDir(""),
m_UseITK(false),
m_UseVTK(false),
m_UseMitkWidgets(false),
m_UseMitkWidgetsExt(false),
m_UseVMTK(false)
{
    m_pMain = (QF::IQF_Main*)app_env::getMainPtr();
    m_pMain->Attach(this);
    setContentView(xmlfile);
}


MainWindow::~MainWindow()
{
}

void MainWindow::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "Generate") == 0)
    {
        Generate();
    }
    else if (strcmp(szMessage, "ScanDirectory") == 0)
    {
        QString directory = QFileDialog::getExistingDirectory(this,
            "Select The Plugin Directory",
            ((QLineEdit*)getViewByID("PluginDir"))->text()
        );
        if (!directory.isEmpty())
        {
            ((QLineEdit*)getViewByID("PluginDir"))->setText(directory);
        }
    }
}

void MainWindow::Generate()
{
    std::cout << "Generate!" << std::endl;
    QLineEdit* lineEdit = (QLineEdit*)getViewByID("PluginName");
    m_PluginName = lineEdit->text();
    lineEdit = (QLineEdit*)getViewByID("ViewName");
    m_ViewName = lineEdit->text();
    lineEdit = (QLineEdit*)getViewByID("PluginDir");
    m_PluginDir = lineEdit->text();

    QMessageBox msgBox;
    if (m_PluginName.isEmpty() || m_PluginDir.isEmpty() || m_ViewName.isEmpty())
    {
        msgBox.setText("Please Confirm The Input Is Not Empty!");
        msgBox.exec();
        return;
    }
    GenerateCMakeList();
    GenerateSourceFiles();
    
    msgBox.setText("The Plugin "+ m_PluginName+" Has Been Generated Successfully!");
    msgBox.exec();

}

void MainWindow::CreateFile(QString fileName, const QString fileContent)
{
  //  CheckAndCreateDirectory(m_PluginDir);
    CheckAndCreateDirectory(m_PluginDir+"/"+m_PluginName);
    QFile file(m_PluginDir + "/" + m_PluginName + "/" + fileName);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
        return;
    std::cout << "Create File "<< fileName.toStdString()<<" Successed!" << std::endl;

    QTextStream out(&file);
    out << fileContent;
}

void MainWindow::GenerateSourceFiles()
{
    QString text = ViewH;
    text.replace("@ViewName@", m_ViewName);
    CreateFile(QString(m_ViewName + ".h"), text);

    text = ViewC;
    text.replace("@ViewName@", m_ViewName);
    CreateFile(QString(m_ViewName + ".cxx"), text);

    text = ViewActivatorH;
    text.replace("@PluginName@", m_PluginName);
    text.replace("@ViewName@", m_ViewName);
    CreateFile(QString(m_PluginName + "Activator.h"), text);

    text = ViewActivatorC;
    text.replace("@PluginName@", m_PluginName);
    text.replace("@ViewName@", m_ViewName);
    CreateFile(QString(m_PluginName + "Activator.cxx"), text);

}

void MainWindow::GenerateCMakeList()
{ 
    m_CMakeListText.clear();
    m_CMakeListText = CMakeListTemplate;

    QCheckBox* checkBox = (QCheckBox*)getViewByID("UseITK");
    m_UseITK = checkBox->isChecked();
    checkBox = (QCheckBox*)getViewByID("UseVTK");
    m_UseVTK = checkBox->isChecked();
    checkBox = (QCheckBox*)getViewByID("UseMitkWidgets");
    m_UseMitkWidgets = checkBox->isChecked();
    checkBox = (QCheckBox*)getViewByID("UseMitkWidgetsExt");
    m_UseMitkWidgetsExt = checkBox->isChecked();
    checkBox = (QCheckBox*)getViewByID("UseVMTK");
    m_UseVMTK = checkBox->isChecked();

    m_CMakeListText.replace("@PluginName@", m_PluginName);
    m_CMakeListText.replace("@QTFRAMEWORK_LIBRARIES@", "${QTFRAMEWORK_LIBRARIES}");
    m_CMakeListText.replace("@QFMAIN_LIBRARIES@", "${QFMAIN_LIBRARIES}");

    m_CMakeListText.replace("@VTK_LIBRARIES@", m_UseVTK ? "${VTK_LIBRARIES}" : "");
    m_CMakeListText.replace("@ITK_LIBRARIES@", m_UseITK ? "${ITK_LIBRARIES}" : "");

    m_CMakeListText.replace("@MitkQtWidgets@", m_UseMitkWidgets ? "MitkQtWidgets" : "");
    m_CMakeListText.replace("@MitkQtWidgetsExt@", m_UseMitkWidgetsExt ? "MitkQtWidgetsExt" : "");

    m_CMakeListText.replace("@VMTK_LIBRARIES@", m_UseVMTK ? "${VMTK_LIBRARIES}" : "");

    std::cout << m_CMakeListText.toStdString() << std::endl;

    CreateFile("CMakeLists.txt", m_CMakeListText);

}

void MainWindow::CheckAndCreateDirectory(const QString path)
{
    QDir dir(path);
    if (dir.exists())
    {
        return;
    }
    else
    {
        bool ok = dir.mkpath(path);
        return;
    }
}





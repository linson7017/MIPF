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
m_ProjectName(""),
m_ModuleName(""),
m_ComName(""),
m_CommandName(""),
m_MessageName(""),
m_Dir(""),
m_UseITK(false),
m_UseVTK(false),
m_UseMitkWidgets(false),
m_UseMitkWidgetsExt(false),
m_UseCom(false),
m_UseCommand(false),
m_UseMessage(false)
{
    m_pMain = (QF::IQF_Main*)app_env::getMainPtr();
    m_pMain->Attach(this);
    setContentView(xmlfile);

    QLineEdit* lineEdit = (QLineEdit*)getViewByID("ModuleName");
    connect(lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(ModuleNameChanged(const QString&)));

}


MainWindow::~MainWindow()
{
}

void MainWindow::ModuleNameChanged(const QString& text)
{
    QLineEdit* lineEdit = (QLineEdit*)getViewByID("ComName");
    lineEdit->setText("CQF_"+text+"Com");
    lineEdit = (QLineEdit*)getViewByID("CommandName");
    lineEdit->setText("CQF_" + text + "Command");
    lineEdit = (QLineEdit*)getViewByID("MessageName");
    lineEdit->setText("CQF_" + text + "Message");
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
            "Select The Module Directory",
            ((QLineEdit*)getViewByID("Directory"))->text()
        );
        if (!directory.isEmpty())
        {
            ((QLineEdit*)getViewByID("Directory"))->setText(directory);
        }
    }
}

void MainWindow::Generate()
{
    std::cout << "Generate!" << std::endl;
    QLineEdit* lineEdit = (QLineEdit*)getViewByID("ModuleName");
    m_ModuleName = lineEdit->text();
    lineEdit = (QLineEdit*)getViewByID("ProjectName");
    m_ProjectName = lineEdit->text();
    lineEdit = (QLineEdit*)getViewByID("ComName");
    m_ComName = lineEdit->text();
    lineEdit = (QLineEdit*)getViewByID("CommandName");
    m_CommandName = lineEdit->text();
    lineEdit = (QLineEdit*)getViewByID("MessageName");
    m_MessageName = lineEdit->text();
    lineEdit = (QLineEdit*)getViewByID("Directory");
    m_Dir = lineEdit->text();

    QCheckBox* checkBox = (QCheckBox*)getViewByID("UseCom");
    m_UseCom = checkBox->isChecked();
    checkBox = (QCheckBox*)getViewByID("UseCommand");
    m_UseCommand = checkBox->isChecked();
    checkBox = (QCheckBox*)getViewByID("UseMessage");
    m_UseMessage = checkBox->isChecked();

    QMessageBox msgBox;
    if (m_ModuleName.isEmpty() || m_Dir.isEmpty()|| m_ProjectName.isEmpty())
    {
        msgBox.setText("Please Confirm The Input Is Not Empty!");
        msgBox.exec();
        return;
    }
    GenerateCMakeList();
    GenerateSourceFiles();
    
    msgBox.setText("The Module "+ m_ModuleName +" Has Been Generated Successfully!");
    msgBox.exec();

}

void MainWindow::CreateFile(QString fileName, const QString fileContent)
{
    CheckAndCreateDirectory(m_Dir +"/"+ m_ProjectName);
    QFile file(m_Dir + "/" + m_ProjectName + "/" + fileName);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
        return;
    std::cout << "Create File "<< fileName.toStdString()<<" Successed!" << std::endl;

    QTextStream out(&file);
    out << fileContent;
}

void MainWindow::GenerateSourceFiles()
{
    QString text = "";
    if (m_UseCom)
    {
        text = ComH;
        text.replace("@ComName@", m_ComName);
        text.replace("@ModuleName@", m_ModuleName);
        if (!m_UseCommand)
        {
            text.replace("class @CommandName@;", "");
            text.replace("@CommandName@* m_pMainCommand;", "");     
        }
        else
        {
            text.replace("@CommandName@", m_CommandName);
        }
        if (!m_UseMessage)
        {
            text.replace("class @MessageName@;", "");
            text.replace("@MessageName@* m_pMainMessage;", "");
        }
        else
        {
            text.replace("@MessageName@", m_MessageName);
        }
        CreateFile(QString(m_ComName + ".h"), text);


        text = ComC;
        text.replace("@ComName@", m_ComName);
        if (!m_UseCommand)
        {
            text.replace("#include \"@CommandName@.h\"", "");
            text.replace("m_pMainCommand->Release();", "");
            text.replace("m_pMainCommand = new @CommandName@(m_pMain);", "");
            text.replace("return m_pMainCommand;", "return NULL;");
        }
        else
        {
            text.replace("@CommandName@", m_CommandName);
        }
        if (!m_UseMessage)
        {
            text.replace("#include \"@MessageName@.h\"", "");
            text.replace("m_pMainMessage->Release();", "");
            text.replace("m_pMainMessage = new @MessageName@(m_pMain);", "");
            text.replace("return m_pMainMessage;", "return NULL;");
        }
        else
        {
            text.replace("@MessageName@", m_MessageName);
        }
        CreateFile(QString( m_ComName + ".cxx"), text);
    }
    
    if (m_UseCommand)
    {
        text = CommandH;
        text.replace("@CommandName@", m_CommandName);
        CreateFile(QString(m_CommandName + ".h"), text);

        text = CommandC;
        text.replace("@CommandName@", m_CommandName);
        CreateFile(QString( m_CommandName + ".cxx"), text);
    }

    if (m_UseMessage)
    {
        text = MessageH;
        text.replace("@MessageName@", m_MessageName);
        CreateFile(QString( m_MessageName + ".h"), text);

        text = MessageC;
        text.replace("@MessageName@", m_MessageName);
        CreateFile(QString(m_MessageName + ".cxx"), text);
    }
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

    m_CMakeListText.replace("@ProjectName@", m_ProjectName);
    m_CMakeListText.replace("@QTFRAMEWORK_LIBRARIES@", "${QTFRAMEWORK_LIBRARIES}");
    m_CMakeListText.replace("@QFMAIN_LIBRARIES@", "${QFMAIN_LIBRARIES}");

    m_CMakeListText.replace("@VTK_LIBRARIES@", m_UseVTK ? "${VTK_LIBRARIES}" : "");
    m_CMakeListText.replace("@ITK_LIBRARIES@", m_UseITK ? "${ITK_LIBRARIES}" : "");

    m_CMakeListText.replace("@MitkQtWidgets@", m_UseMitkWidgets ? "MitkQtWidgets" : "");
    m_CMakeListText.replace("@MitkQtWidgetsExt@", m_UseMitkWidgetsExt ? "MitkQtWidgetsExt" : "");

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





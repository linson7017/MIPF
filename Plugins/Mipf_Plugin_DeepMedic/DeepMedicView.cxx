#include "DeepMedicView.h" 
#include "iqf_main.h"  

#include <Python.h>
  
DeepMedicView::DeepMedicView() :MitkPluginView() 
{
}
 
DeepMedicView::~DeepMedicView() 
{
}
 
void DeepMedicView::CreateView()
{
    m_ui.setupUi(this);
    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &DeepMedicView::Apply);
} 
 
WndHandle DeepMedicView::GetPluginHandle() 
{
    return this; 
}

void DeepMedicView::Apply()
{
    QString deepMedicRunfile = "S:\\Python\\DP\\deepmedic\\deepMedicRun";
    QString deepMedicPath = "S:/Python/DP/deepmedic";
    QString command = "python " + deepMedicRunfile + " -dev gpu -test " + deepMedicPath + "/examples/configFiles/myDeepMedic/test/testConfig.cfg -model " \
       + deepMedicPath + "/examples/myoutput/cnnModels/myTrainSessionDeepMedic/myDeepMedic.myTrainSessionDeepMedic.2017-12-07.08.32.52.849000.save";
    //system("activate python2.7");
    system(command.toLocal8Bit().constData());

    /* Py_Initialize();
     if (!Py_IsInitialized())
         return;
     PyRun_SimpleString("print(123)");*/
}
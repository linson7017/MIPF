#include "Mipf_Plugin_SurfaceCutActivator.h"


#include "SurfaceCutView.h"
#include "SurfaceConnectedView.h"
#include "SurfaceCombineView.h"

#include "Res/R.h"

#include "Utils/QObjectFactory.h"

#include <usModuleInitialization.h>
#include <usGetModuleContext.h>

US_INITIALIZE_MODULE

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_SurfaceCut_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_SurfaceCut_Activator_ID[] = "Mipf_Plugin_SurfaceCut_Activator_ID";

Mipf_Plugin_SurfaceCut_Activator::Mipf_Plugin_SurfaceCut_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_SurfaceCut_Activator::Init()
{
    return true; 
}

const char* Mipf_Plugin_SurfaceCut_Activator::GetID()
{
    return Mipf_Plugin_SurfaceCut_Activator_ID; 
}

void Mipf_Plugin_SurfaceCut_Activator::Register()
{
    REGISTER_QOBJECT("SurfaceCutWidget", SurfaceCutView);
    REGISTER_QOBJECT("SurfaceConnectedWidget", SurfaceConnectedView);
    REGISTER_QOBJECT("SurfaceCombineWidget", SurfaceCombineView);

}
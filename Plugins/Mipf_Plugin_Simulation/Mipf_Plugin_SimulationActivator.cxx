#include "Mipf_Plugin_SimulationActivator.h"
#include "WireSimulationView.h" 
#include "WireSimulationPBDView.h"
#include "WireMouldingTestView.h"

#include "Utils/PluginFactory.h" 

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_Simulation_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_Simulation_Activator_ID[] = "Mipf_Plugin_Simulation_Activator_ID";

Mipf_Plugin_Simulation_Activator::Mipf_Plugin_Simulation_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_Simulation_Activator::Init()
{
    return true; 
}

const char* Mipf_Plugin_Simulation_Activator::GetID()
{
    return Mipf_Plugin_Simulation_Activator_ID; 
}

void Mipf_Plugin_Simulation_Activator::Register()
{
    REGISTER_PLUGIN("WireSimulationWidget", WireSimulationView);
    REGISTER_PLUGIN("WireSimulationPBDWidget", WireSimulationPBDView);
    REGISTER_PLUGIN("WireMouldingWidget", WireMouldingTestView);
}
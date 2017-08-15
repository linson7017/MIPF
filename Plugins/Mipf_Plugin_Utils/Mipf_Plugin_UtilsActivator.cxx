#include "Mipf_Plugin_UtilsActivator.h"

#include "LargestConnectedComponentView.h"
#include "MaskImageView.h"
#include "ImageHoleFillingView.h"
#include "SurfaceExtractView.h"
#include "PointListView.h"
#include "BoneExtract.h"
#include "LankMarkExtractView.h"
#include "SkinExtractView.h"
#include "TransformNodeView.h"

#include "Res/R.h"
#include "Utils/QObjectFactory.h"
#include "QmitkLabelSetWidget.h"

QF_API QF::IQF_Activator* QF::QF_CreatePluginActivator(QF::IQF_Main* pMain)
{
    QF::IQF_Activator* pActivator = new Mipf_Plugin_Utils_Activator(pMain);
    //assert(pActivator);
    return pActivator;
}

const char Mipf_Plugin_Utils_Activator_ID[] = "Mipf_Plugin_Utils_Activator_ID";

Mipf_Plugin_Utils_Activator::Mipf_Plugin_Utils_Activator(QF::IQF_Main* pMain):ActivatorBase(pMain)
{
   
}

bool Mipf_Plugin_Utils_Activator::Init()
{
    return true; 
}

const char* Mipf_Plugin_Utils_Activator::GetID()
{
    return Mipf_Plugin_Utils_Activator_ID;
}

void Mipf_Plugin_Utils_Activator::Register(R* pR)
{
    REGISTER_CLASS("LargestConnectedComponentExtractWidget", LargestConnectedComponentView);
    REGISTER_CLASS("SurfaceExtractWidget", SurfaceExtractView);
    REGISTER_CLASS("MaskImageWidget", MaskImageView);
    REGISTER_CLASS("ImageHoleFillingWidget", ImageHoleFillingView);
    REGISTER_CLASS("PointListWidget", PointListView);
    REGISTER_CLASS("BoneExtractWidget", BoneExtract);
    REGISTER_CLASS("LandMarkExtractWidget", LankMarkExtractView);
    REGISTER_CLASS("SkinExtractWidget", SkinExtractView);
    REGISTER_CLASS("TransformNodeWidget", TransformNodeView);
}

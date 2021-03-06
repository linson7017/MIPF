#include "Mipf_Plugin_UtilsActivator.h"

#include "LargestConnectedComponentView.h"
#include "MaskImageView.h"
#include "ImageHoleFillingView.h"
#include "SurfaceExtractView.h"
#include "PointListView.h"
#include "LankMarkExtractView.h"
#include "TransformNodeView.h"
#include "GeometryInformationView.h"
#include "SurfaceToImageView.h"
#include "ImageInteractionView.h"
#include "ThresholdImageView.h"
#include "SurfaceInteractionView.h"
#include "SurfaceInformationView.h"
#include "LabelImageProcessView.h"
#include "SurfaceIntersectView.h"
#include "ExtractImageSliceView.h"

#include "Res/R.h"
#include "Utils/QObjectFactory.h"
#include "Utils/PluginFactory.h"
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

void Mipf_Plugin_Utils_Activator::Register()
{
    REGISTER_QOBJECT("LargestConnectedComponentExtractWidget", LargestConnectedComponentView);
    REGISTER_QOBJECT("SurfaceExtractWidget", SurfaceExtractView);
    REGISTER_QOBJECT("MaskImageWidget", MaskImageView);
    REGISTER_QOBJECT("ImageHoleFillingWidget", ImageHoleFillingView);
    REGISTER_QOBJECT("PointListWidget", PointListView);
    REGISTER_QOBJECT("LandMarkExtractWidget", LankMarkExtractView);
    REGISTER_QOBJECT("TransformNodeWidget", TransformNodeView);

    REGISTER_PLUGIN("GeometryInformationWidget", GeometryInformationView);
    REGISTER_PLUGIN("SurfaceToImageWidget", SurfaceToImageView);
    REGISTER_PLUGIN("ImageInteractionWidget", ImageInteractionView);
    REGISTER_PLUGIN("ThresholdImageWidget", ThresholdImageView);
    REGISTER_PLUGIN("SurfaceInteractionWidget", SurfaceInteractionView);

    REGISTER_PLUGIN("SurfaceInformationWidget", SurfaceInformationView);
    REGISTER_PLUGIN("LabelImageProcessWidget", LabelImageProcessView);
    REGISTER_PLUGIN("SurfaceIntersectWidget", SurfaceIntersectView);
    REGISTER_PLUGIN("ExtractImageSliceWidget", ExtractImageSliceView);

}

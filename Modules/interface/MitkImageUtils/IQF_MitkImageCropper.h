#ifndef IQF_MitkImageCropper_h__
#define IQF_MitkImageCropper_h__

#include "iconfig.h"

const char QF_MitkImageUtils_ImageCropper[] = "QF_MitkImageUtils_ImageCropper";

namespace mitk
{
    class  DataNode;
    class  GeometryData;
}

class IQF_MitkImageCropper
{
    BUILD_INTERFACE(IQF_MitkImageCropper)
public:
    virtual void CreateBoundingBoxNode(mitk::DataNode* pInNode, mitk::DataNode*pOutNode, const char* szName = "", bool bAutoUse = true)=0;
    virtual void EnableInteraction(bool bEnable = true)=0;
    virtual void SetDataNode(mitk::DataNode* pNode)=0;
    virtual mitk::DataNode::Pointer ProcessImage(mitk::DataNode* pDataNode, mitk::GeometryData* pCropGeometry, const char* szName, bool mask)=0;
};




#endif // IQF_MitkImageCropper_h__

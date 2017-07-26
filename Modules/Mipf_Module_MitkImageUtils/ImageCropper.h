#ifndef ImageCropper_h__
#define ImageCropper_h__

#pragma once
#include "mitkDataNode.h"
#include "mitkBoundingShapeInteractor.h"
#include "mitkGeometryData.h"

#include "MitkImageUtils/IQF_MitkImageCropper.h"

namespace QF
{
    class IQF_Main;
}

class ImageCropper :public IQF_MitkImageCropper
{
public:
    ImageCropper(QF::IQF_Main* pMain);
    ~ImageCropper();

    virtual void CreateBoundingBoxNode(mitk::DataNode* pInNode,mitk::DataNode*pOutNode,const char* szName="",bool bAutoUse = true);
    virtual void EnableInteraction(bool bEnable = true);
    virtual void SetDataNode(mitk::DataNode* pNode);
    virtual mitk::DataNode::Pointer ProcessImage(mitk::DataNode* pDataNode, mitk::GeometryData* pCropGeometry, const char* szName,bool mask);
protected:
    void CreateBoundingShapeInteractor(bool rotationEnabled);
    mitk::Geometry3D::Pointer ImageCropper::InitializeWithImageGeometry(mitk::BaseGeometry::Pointer geometry);
private:
    QF::IQF_Main* m_pMain;
    mitk::BoundingShapeInteractor::Pointer m_BoundingShapeInteractor;
};

#endif // ImageCropper_h__

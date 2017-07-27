#ifndef IQF_MitkSegmentation_h__
#define IQF_MitkSegmentation_h__

const char QF_MitkSegmentation_Tool[] = "QF_MitkSegmentation_Tool";

struct SegRGBColor
{
    SegRGBColor() 
    {
        rgb[0] = 1.0; rgb[1] = 1.0; rgb[2] = 1.0;
    }
    SegRGBColor(float pR,float pG,float pB)
    {
        rgb[0] = pR; rgb[1] = pG; rgb[2] = pB;
    }
    float* data() { return rgb; }
    float rgb[3];
};

namespace mitk
{
    class DataNode;
    class ToolManager;
    class Image;
    class Tool;
}

class IQF_MitkSegmentationTool
{
public:
    virtual void Initialize() = 0;
    virtual void Deinitialize() = 0;
    virtual bool CreateSegmentationNode(const mitk::DataNode* pRefNode, mitk::DataNode* pSegmentationNode, const char* szName, SegRGBColor& rgbColor = SegRGBColor(1.0, 0.0, 0.0)) = 0;
    virtual bool CreateSegmentationNode(const mitk::Image* pOriginImage, mitk::DataNode* pSegmentationNode, const char* szName, SegRGBColor& rgbColor = SegRGBColor(1.0, 0.0, 0.0)) = 0;
    virtual bool CreateLabelSetImageNode(const mitk::DataNode* pRefNode, mitk::DataNode* pLabelSetNode, const char* szName)=0;
    virtual bool CreateLabelSetImageNode(const mitk::Image* pOriginImage, mitk::DataNode* pLabelSetNode, const char* szName)=0;
    virtual mitk::ToolManager* GetToolManager() = 0;
    virtual void SetReferenceData(const mitk::DataNode* node) = 0;
    virtual void SetWorkingData(const mitk::DataNode* node) = 0;
    virtual mitk::DataNode* GetReferenceData() = 0;
    virtual mitk::DataNode* GetWorkingData() = 0;
    virtual mitk::Tool* ChangeTool(const char* szToolID) = 0;
    virtual mitk::Tool* GetActivedTool() = 0;
    virtual void SetSurfaceInterpolateOn(bool bEnableInterpolate=true) = 0;
};

#endif // IQF_MitkSegmentation_h__

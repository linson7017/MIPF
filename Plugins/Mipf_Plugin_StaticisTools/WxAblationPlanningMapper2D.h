#pragma once

#include <QString>

#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>

#include <mitkCommon.h>
#include <mitkGLMapper2D.h>
#include <mitkSurface.h>
#include <mitkGL.h>
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkSurface.h"
#include <mitkLine.h>


namespace mitk {

class  WxAblationPlanningMapper2D : public GLMapper
{
public:
    mitkClassMacro(WxAblationPlanningMapper2D, GLMapper);

    itkNewMacro(Self);

    const Surface* GetInput(void);

    virtual void Paint(BaseRenderer* renderer);

    itkSetConstObjectMacro(Surface, Surface);

    itkGetConstObjectMacro(Surface, Surface);

    void SetDataNode( DataNode::Pointer node );

    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false);

    virtual void ApplyProperties(BaseRenderer* renderer);

	//============================================================================
	bool IsOn2DLine(mitk::Point2D begin,mitk::Point2D end,mitk::Point2D cursor2d,float error);

	void DrawTriangle(mitk::Point2D point_1, mitk::Point2D point_2, mitk::Point2D point_3, float * rgba, bool bSolid);
	void DrawRectangle(mitk::Point2D point_1, mitk::Point2D point_2, mitk::Point2D point_3,  mitk::Point2D point_4, 
		mitk::Point2D point_12Mid,  mitk::Point2D point_34Mid, float * rgba, bool bSolid);

	void AddExcludeWidget(QString widgetName);
	void RemoveExcludeWidget(QString widgetName);

	std::vector<QString> strExcludeNameList;
	//============================================================================

protected:

    WxAblationPlanningMapper2D();

    virtual ~WxAblationPlanningMapper2D();

    Surface::ConstPointer m_Surface;

};
}

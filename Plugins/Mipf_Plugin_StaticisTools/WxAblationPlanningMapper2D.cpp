#include "WxAblationPlanningMapper2D.h"

mitk::WxAblationPlanningMapper2D::WxAblationPlanningMapper2D()
{

}

mitk::WxAblationPlanningMapper2D::~WxAblationPlanningMapper2D()
{

}

void mitk::WxAblationPlanningMapper2D::AddExcludeWidget(QString widgetName)
{
    strExcludeNameList.push_back(widgetName);
}

void mitk::WxAblationPlanningMapper2D::RemoveExcludeWidget(QString widgetName)
{
    std::vector<QString>::iterator iter = strExcludeNameList.begin();
    for (;	iter != strExcludeNameList.end();
         ++iter)
    {
        if ((*iter) == widgetName)
        {
            strExcludeNameList.erase(iter);
        }
    }
}

const mitk::Surface *mitk::WxAblationPlanningMapper2D::GetInput(void)
{
    if(m_Surface.IsNotNull())
        return m_Surface;

    return static_cast<const Surface * > ( GetData() );
}

void mitk::WxAblationPlanningMapper2D::SetDataNode( mitk::DataNode::Pointer node )
{
    Superclass::SetDataNode( node );

    bool useCellData;

    if (dynamic_cast<BoolProperty *>(node->GetProperty("deprecated useCellDataForColouring")) == NULL)
    {
        useCellData = false;
    }
    else
    {
        useCellData = dynamic_cast<BoolProperty *>(node->GetProperty("deprecated useCellDataForColouring"))->GetValue();
    }

    if (!useCellData)
    {
        // search min/max point scalars over all time steps
        double dataRange[2] = {0,0};
        double range[2];

        Surface::Pointer input  = const_cast< Surface* >(dynamic_cast<const Surface*>( this->GetDataNode()->GetData() ));
        if(input.IsNull()) return;
        const TimeGeometry::Pointer inputTimeGeometry = input->GetTimeGeometry();
        if(( inputTimeGeometry.IsNull() ) || ( inputTimeGeometry->GetTimeStamp() == 0 ) ) return;
        for (unsigned int timestep=0; timestep<inputTimeGeometry->GetTimeStamp(); timestep++)
        {
            vtkPolyData * vtkpolydata = input->GetVtkPolyData( timestep );
            if((vtkpolydata==NULL) || (vtkpolydata->GetNumberOfPoints() < 1 )) continue;
            vtkDataArray *vpointscalars = vtkpolydata->GetPointData()->GetScalars();
            if (vpointscalars) {
                vpointscalars->GetRange( range, 0 );
                if (dataRange[0]==0 && dataRange[1]==0) {
                    dataRange[0] = range[0];
                    dataRange[1] = range[1];
                }
                else {
                    if (range[0] < dataRange[0]) dataRange[0] = range[0];
                    if (range[1] > dataRange[1]) dataRange[1] = range[1];
                }
            }
        }
    }
}

void mitk::WxAblationPlanningMapper2D::Paint(mitk::BaseRenderer * renderer)
{
    MITK_INFO << "Paint";
    //=========================选择性投影===========================
    std::vector<QString>::iterator iter = strExcludeNameList.begin();
    for (;	iter != strExcludeNameList.end();
         ++iter)
    {
        if ((*iter) == renderer->GetName())
        {
            return;
        }
    }
    //==============================================================
    if(IsVisible(renderer)==false) return;

    Surface::Pointer input  = const_cast<Surface*>(this->GetInput());

    if(input.IsNull())
        return;

    // get the TimeSlicedGeometry of the input object
    const TimeGeometry* inputTimeGeometry = input->GetTimeGeometry();
    if(( inputTimeGeometry == NULL ) || ( inputTimeGeometry->GetTimeStamp() == 0 ) )
        return;

    // get the world time
    mitk::PlaneGeometry::ConstPointer worldGeometry = renderer->GetCurrentWorldPlaneGeometry();   //获取当前切面的世界平面几何模型
    assert( worldGeometry.IsNotNull() );

    //itk::TimeStamp time = worldGeometry->GetTimeStamp();
    //int timestep = 0;

    ////if (time > itk::NumericTraits::NonpositiveMin())
    //    timestep = inputTimeGeometry->TimePointToTimeStep(time);

    //if (inputTimeGeometry->IsValidTimeStep(timestep) == false)
    //    return;

    //==================================投影=====================================
    mitk::Vector3D needleDirection = input->GetUpdatedGeometry()->GetAxisVector(2);  //针指向为Z方向
    mitk::Vector3D needleWide = input->GetGeometry()->GetAxisVector(0);
    mitk::Point3D needleOrigin = input->GetGeometry()->GetOrigin();  //原点位于针尖
    mitk::Point3D needleEnd = needleOrigin - needleDirection;
    mitk::Point2D needleOrigin2d, needleEnd2d;

    mitk::PlaneGeometry::ConstPointer worldPlaneGeometry = dynamic_cast<const PlaneGeometry*>(worldGeometry.GetPointer());
    worldGeometry->Map(needleEnd,needleEnd2d);               //针柄世界平面坐标
    worldGeometry->Map(needleOrigin,needleOrigin2d);     //针尖世界平面坐标

    mitk::PlaneGeometry::ConstPointer planeGeometry = renderer->GetSliceNavigationController()->GetCurrentPlaneGeometry();
    planeGeometry->Project(needleEnd, needleEnd);
    planeGeometry->Project(needleOrigin, needleOrigin);

    renderer->WorldToDisplay(needleEnd, needleOrigin2d);
    renderer->WorldToDisplay(needleOrigin, needleEnd2d);

    //DisplayGeometry* displayGeometry=renderer->GetDisplayGeometry();
    //displayGeometry->WorldToDisplay(needleOrigin2d, needleOrigin2d);    //针柄显示平面坐标
    //displayGeometry->WorldToDisplay(needleEnd2d, needleEnd2d);           //针尖显示平面坐标

    mitk::Vector2D needleDirection2d=needleEnd2d - needleOrigin2d;     //针柄到针尖的显示平面向量
    float needlleLength=needleDirection2d.GetNorm();
    needleDirection2d.Normalize();   //二维向量归一化
    mitk::Vector2D needleWideDir2d;
    //针的宽度方向与针的指向垂直，这两步使得向量逆时针旋转90度
    //逆时针旋转a角度的旋转矩阵为[cos(a)  -sin(a) ,    sin(a)   cos(a)]
    needleWideDir2d[0] = -needleDirection2d[1];
    needleWideDir2d[1] = needleDirection2d[0];
    needleWideDir2d.Normalize();

    float f, h;    //根据针尖点和针柄点到平面的距离决定针的哪一端大，哪一端小
    if (worldPlaneGeometry->SignedDistanceFromPlane(needleEnd) < worldPlaneGeometry->SignedDistanceFromPlane(needleOrigin))
    {
        f = needleWide.GetNorm()*0.2;  //针柄宽度放大因子
        h = needleWide.GetNorm()*0.4;  //针尖宽度放大因子
    }
    else
    {
        f = needleWide.GetNorm()*0.4;  //针柄宽度放大因子
        h = needleWide.GetNorm()*0.2;  //针尖宽度放大因子
    }

    mitk::Point2D handle2d = needleOrigin2d + needleDirection2d*needlleLength*0.05;     //针尖部分所占整根针长度的百分比，0.05 = 5%
    mitk::Point2D e2d1 = needleEnd2d + needleWideDir2d*f;
    mitk::Point2D e2d2 = needleEnd2d-needleWideDir2d*f;
    mitk::Point2D o2d1 = handle2d+needleWideDir2d*h;
    mitk::Point2D o2d2 = handle2d-needleWideDir2d*h;

    //apply color and opacity read from the PropertyList
    ApplyProperties(renderer);

    float rgba[4] = {1.0f,1.0f,1.0f,1.0f};
    // check for color prop and use it for rendering if it exists
    GetColor(rgba, renderer);
    // check for opacity prop and use it for rendering if it exists
    GetOpacity(rgba[3], renderer);

    //计算针与平面的交叉点
    mitk::Line3D needleLine(needleOrigin, needleDirection);
    mitk::Point3D IntersectionPt;
    //计算线(直线，非线段)同平面的交点
    worldPlaneGeometry->IntersectionPoint(needleLine, IntersectionPt);

    mitk::Point2D IntersectionPt2d;
    worldGeometry->Map(IntersectionPt, IntersectionPt2d);               //交叉点世界平面坐标
    //displayGeometry->WorldToDisplay(IntersectionPt2d, IntersectionPt2d);//交叉点显示平面坐标
    planeGeometry->WorldToIndex(IntersectionPt2d, IntersectionPt2d);
    //===============================================================

    glShadeModel(GL_SMOOTH);

    if (!IsOn2DLine(handle2d, needleEnd2d, IntersectionPt2d, 3))    //如果交叉点不在线段上，则针与面无交点
    {
        if (worldPlaneGeometry->SignedDistanceFromPlane(needleEnd) > 0)
            rgba[3] = 0.9;
        else
            rgba[3] = 0.3;
        //========================针杆==========================
        DrawRectangle(e2d1, e2d2, o2d2, o2d1, needleEnd2d, handle2d, rgba, true);
        //========================针尖==========================
        DrawTriangle(o2d1, o2d2, needleOrigin2d, rgba, true);
        //======================================================
    }
    else//如果交叉点在线段上，则针穿越了面
    {
        //计算交点到针柄的距离
        mitk::Vector2D lineDirection2d=IntersectionPt2d - needleEnd2d;
        float  lineleLength=lineDirection2d.GetNorm();

        f = needleWide.GetNorm() * 0.2* (1 + lineleLength / needlleLength);

        mitk::Point2D IntersectionPtd1 = IntersectionPt2d + needleWideDir2d*f;
        mitk::Point2D IntersectionPtd2 = IntersectionPt2d - needleWideDir2d*f;
        //=================根据点与切面的关系设置透明度=================
        if (worldPlaneGeometry->SignedDistanceFromPlane(needleEnd) > 0)
            rgba[3] = 0.9;
        else
            rgba[3] = 0.3;
        //========================针杆==========================
        DrawRectangle(e2d1, e2d2, IntersectionPtd2, IntersectionPtd1, needleEnd2d, IntersectionPt2d, rgba, true);

        if (worldPlaneGeometry->SignedDistanceFromPlane(needleEnd) > 0)
            rgba[3] = 0.3;
        else
            rgba[3] = 0.9;
        DrawRectangle(IntersectionPtd1, IntersectionPtd2, o2d2, o2d1, IntersectionPt2d, handle2d, rgba, true);
        //========================针尖==========================
        DrawTriangle(o2d1, o2d2, needleOrigin2d, rgba, true);
    }
}

void mitk::WxAblationPlanningMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
    node->AddProperty( "layer", mitk::IntProperty::New(100), renderer, overwrite);
    Superclass::SetDefaultProperties(node, renderer, overwrite);
}

void mitk::WxAblationPlanningMapper2D::ApplyProperties(mitk::BaseRenderer* renderer)
{
    Superclass::ApplyProperties(renderer);
}


bool mitk::WxAblationPlanningMapper2D::IsOn2DLine(mitk::Point2D begin,mitk::Point2D end,mitk::Point2D cursor2d,float error)
{
    Line<ScalarType,2> *needle = new Line<ScalarType,2>();
    needle->SetPoints(begin, end);
    float length=(begin-end).GetNorm();
    float thisDistance = needle->Distance(cursor2d);
    float disToEnd=(end-needle->Project(cursor2d)).GetNorm();
    float disToOri=(begin-needle->Project(cursor2d)).GetNorm();

    if (thisDistance<error && disToEnd<=length && disToOri<=length)
        return true;
    else
        return false;
    delete needle;
}

void mitk::WxAblationPlanningMapper2D::DrawTriangle(mitk::Point2D point_1, mitk::Point2D point_2, mitk::Point2D point_3, float * rgba, bool bSolid)
{
    if (bSolid)
    {
        glBegin (GL_POLYGON);
        glColor4f(rgba[0]*0.75,rgba[1]*0.75,rgba[2]*0.75,rgba[3]);
        glVertex2f(point_1[0], point_1[1]);
        glVertex2f(point_2[0], point_2[1]);
        glColor4fv(rgba);
        glVertex2f(point_3[0], point_3[1]);
        glEnd();
    }

    glBegin (GL_LINE_LOOP);
    glColor4f(0,0,0,1);
    glVertex2f(point_3[0], point_3[1]);
    glVertex2f(point_1[0], point_1[1]);
    glVertex2f(point_2[0], point_2[1]);
    glColor4fv(rgba);
    glEnd();
}


void mitk::WxAblationPlanningMapper2D::DrawRectangle(mitk::Point2D point_1, mitk::Point2D point_2, mitk::Point2D point_3,  mitk::Point2D point_4,
                                                  mitk::Point2D point_12Mid,  mitk::Point2D point_34Mid, float * rgba, bool bSolid)
{
    if (bSolid)
    {
        glBegin (GL_POLYGON);
        glColor4f(rgba[0]*0.5,rgba[1]*0.5,rgba[2]*0.5,rgba[3]);
        glVertex2f(point_2[0], point_2[1]);
        glVertex2f(point_3[0], point_3[1]);
        glColor4fv(rgba);
        glVertex2f(point_34Mid[0], point_34Mid[1]);
        glVertex2f(point_12Mid[0], point_12Mid[1]);
        glEnd();

        glBegin (GL_POLYGON);
        glColor4f(rgba[0]*0.5,rgba[1]*0.5,rgba[2]*0.5,rgba[3]);
        glVertex2f(point_1[0], point_1[1]);
        glVertex2f(point_4[0], point_4[1]);
        glColor4fv(rgba);
        glVertex2f(point_34Mid[0], point_34Mid[1]);
        glVertex2f(point_12Mid[0], point_12Mid[1]);
        glEnd();
    }

    glBegin (GL_LINE_LOOP);
    glColor4f(0,0,0,1);
    glVertex2f(point_1[0], point_1[1]);
    glVertex2f(point_2[0], point_2[1]);
    glVertex2f(point_3[0], point_3[1]);
    glVertex2f(point_4[0], point_4[1]);
    glEnd();
}

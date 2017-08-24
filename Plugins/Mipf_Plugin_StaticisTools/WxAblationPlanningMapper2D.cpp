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
    //=========================ѡ����ͶӰ===========================
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
    mitk::PlaneGeometry::ConstPointer worldGeometry = renderer->GetCurrentWorldPlaneGeometry();   //��ȡ��ǰ���������ƽ�漸��ģ��
    assert( worldGeometry.IsNotNull() );

    //itk::TimeStamp time = worldGeometry->GetTimeStamp();
    //int timestep = 0;

    ////if (time > itk::NumericTraits::NonpositiveMin())
    //    timestep = inputTimeGeometry->TimePointToTimeStep(time);

    //if (inputTimeGeometry->IsValidTimeStep(timestep) == false)
    //    return;

    //==================================ͶӰ=====================================
    mitk::Vector3D needleDirection = input->GetUpdatedGeometry()->GetAxisVector(2);  //��ָ��ΪZ����
    mitk::Vector3D needleWide = input->GetGeometry()->GetAxisVector(0);
    mitk::Point3D needleOrigin = input->GetGeometry()->GetOrigin();  //ԭ��λ�����
    mitk::Point3D needleEnd = needleOrigin - needleDirection;
    mitk::Point2D needleOrigin2d, needleEnd2d;

    mitk::PlaneGeometry::ConstPointer worldPlaneGeometry = dynamic_cast<const PlaneGeometry*>(worldGeometry.GetPointer());
    worldGeometry->Map(needleEnd,needleEnd2d);               //�������ƽ������
    worldGeometry->Map(needleOrigin,needleOrigin2d);     //�������ƽ������

    mitk::PlaneGeometry::ConstPointer planeGeometry = renderer->GetSliceNavigationController()->GetCurrentPlaneGeometry();
    planeGeometry->Project(needleEnd, needleEnd);
    planeGeometry->Project(needleOrigin, needleOrigin);

    renderer->WorldToDisplay(needleEnd, needleOrigin2d);
    renderer->WorldToDisplay(needleOrigin, needleEnd2d);

    //DisplayGeometry* displayGeometry=renderer->GetDisplayGeometry();
    //displayGeometry->WorldToDisplay(needleOrigin2d, needleOrigin2d);    //�����ʾƽ������
    //displayGeometry->WorldToDisplay(needleEnd2d, needleEnd2d);           //�����ʾƽ������

    mitk::Vector2D needleDirection2d=needleEnd2d - needleOrigin2d;     //�����������ʾƽ������
    float needlleLength=needleDirection2d.GetNorm();
    needleDirection2d.Normalize();   //��ά������һ��
    mitk::Vector2D needleWideDir2d;
    //��Ŀ�ȷ��������ָ��ֱ��������ʹ��������ʱ����ת90��
    //��ʱ����תa�Ƕȵ���ת����Ϊ[cos(a)  -sin(a) ,    sin(a)   cos(a)]
    needleWideDir2d[0] = -needleDirection2d[1];
    needleWideDir2d[1] = needleDirection2d[0];
    needleWideDir2d.Normalize();

    float f, h;    //�������������㵽ƽ��ľ�����������һ�˴���һ��С
    if (worldPlaneGeometry->SignedDistanceFromPlane(needleEnd) < worldPlaneGeometry->SignedDistanceFromPlane(needleOrigin))
    {
        f = needleWide.GetNorm()*0.2;  //�����ȷŴ�����
        h = needleWide.GetNorm()*0.4;  //����ȷŴ�����
    }
    else
    {
        f = needleWide.GetNorm()*0.4;  //�����ȷŴ�����
        h = needleWide.GetNorm()*0.2;  //����ȷŴ�����
    }

    mitk::Point2D handle2d = needleOrigin2d + needleDirection2d*needlleLength*0.05;     //��ⲿ����ռ�����볤�ȵİٷֱȣ�0.05 = 5%
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

    //��������ƽ��Ľ����
    mitk::Line3D needleLine(needleOrigin, needleDirection);
    mitk::Point3D IntersectionPt;
    //������(ֱ�ߣ����߶�)ͬƽ��Ľ���
    worldPlaneGeometry->IntersectionPoint(needleLine, IntersectionPt);

    mitk::Point2D IntersectionPt2d;
    worldGeometry->Map(IntersectionPt, IntersectionPt2d);               //���������ƽ������
    //displayGeometry->WorldToDisplay(IntersectionPt2d, IntersectionPt2d);//�������ʾƽ������
    planeGeometry->WorldToIndex(IntersectionPt2d, IntersectionPt2d);
    //===============================================================

    glShadeModel(GL_SMOOTH);

    if (!IsOn2DLine(handle2d, needleEnd2d, IntersectionPt2d, 3))    //�������㲻���߶��ϣ����������޽���
    {
        if (worldPlaneGeometry->SignedDistanceFromPlane(needleEnd) > 0)
            rgba[3] = 0.9;
        else
            rgba[3] = 0.3;
        //========================���==========================
        DrawRectangle(e2d1, e2d2, o2d2, o2d1, needleEnd2d, handle2d, rgba, true);
        //========================���==========================
        DrawTriangle(o2d1, o2d2, needleOrigin2d, rgba, true);
        //======================================================
    }
    else//�����������߶��ϣ����봩Խ����
    {
        //���㽻�㵽����ľ���
        mitk::Vector2D lineDirection2d=IntersectionPt2d - needleEnd2d;
        float  lineleLength=lineDirection2d.GetNorm();

        f = needleWide.GetNorm() * 0.2* (1 + lineleLength / needlleLength);

        mitk::Point2D IntersectionPtd1 = IntersectionPt2d + needleWideDir2d*f;
        mitk::Point2D IntersectionPtd2 = IntersectionPt2d - needleWideDir2d*f;
        //=================���ݵ�������Ĺ�ϵ����͸����=================
        if (worldPlaneGeometry->SignedDistanceFromPlane(needleEnd) > 0)
            rgba[3] = 0.9;
        else
            rgba[3] = 0.3;
        //========================���==========================
        DrawRectangle(e2d1, e2d2, IntersectionPtd2, IntersectionPtd1, needleEnd2d, IntersectionPt2d, rgba, true);

        if (worldPlaneGeometry->SignedDistanceFromPlane(needleEnd) > 0)
            rgba[3] = 0.3;
        else
            rgba[3] = 0.9;
        DrawRectangle(IntersectionPtd1, IntersectionPtd2, o2d2, o2d1, IntersectionPt2d, handle2d, rgba, true);
        //========================���==========================
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

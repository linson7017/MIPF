#include "WireMouldingTestView.h"
#include "mitkPointSet.h"
#include "mitkVtkScalarModeProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"

#include "vtkLookupTable.h"

//vtkPolydata
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkAppendPolyData.h>
#include <vtkArrowSource.h>
#include <vtkMath.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkMutableDirectedGraph.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkTransform.h>
#include <vtkSphereSource.h>
#include<vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkGenericCell.h>
#include <vtkCell.h>
#include <vtkPolyLine.h>
#include <vtkTubeFilter.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkKochanekSpline.h>
#include <vtkFillHolesFilter.h>
#include <vtkPlane.h>
#include <vtkClipPolyData.h>
#include <vtkFeatureEdges.h>
#include <vtkStripper.h>
#include <vtkCleanPolyData.h>
#include <vtkTriangleFilter.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkvmtkCapPolyData.h>
#include <ctkSliderWidget.h>
#include "Rendering/ColoredSurfaceVtkMapper.h"


#include "Core/IQF_ObjectFactory.h"


WireMouldingTestView::WireMouldingTestView()
{
    
}


WireMouldingTestView::~WireMouldingTestView()
{

}

void WireMouldingTestView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreateSurfacePredicate());
    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &WireMouldingTestView::Apply);

    
}

void WireMouldingTestView::Apply()
{
    IQF_ObjectFactory* pFactory = (IQF_ObjectFactory*)GetInterfacePtr(QF_Core_ObjectFactory);
    if (!pFactory)
    {
        return;
    }
    IQF_GuideWireMoulding* pGuideWireMoulding = (IQF_GuideWireMoulding*)pFactory->CreateObject(Object_ID_GuideWireMoulding);
    if (!pGuideWireMoulding)
    {
        return;
    }
    mitk::PointSet* ljlxPS = dynamic_cast<mitk::PointSet*>(GetDataStorage()->GetNamedNode("ljlx")->GetData());
    //m_ljPoint = ljlxPS->GetPoint(1);
    //m_lxPoint = ljlxPS->GetPoint(3);
    mitk::PointSet* rkPS = dynamic_cast<mitk::PointSet*>(GetDataStorage()->GetNamedNode("rk")->GetData());
    //m_entryPoint = rkPS->GetPoint(0);
    //m_entryDirectionPoint = rkPS->GetPoint(1);

    mitk::Surface* centerline = dynamic_cast<mitk::Surface*>(GetDataStorage()->GetNamedNode("tumorVesselCenterline")->GetData());
    pGuideWireMoulding->SetCenterLine(centerline->GetVtkPolyData(),m_ui.InvertCB->isChecked());
    vtkPoints* points = centerline->GetVtkPolyData()->GetPoints();
    double centerLineLength = 0.0;
    for (vtkIdType i = 1; i < points->GetNumberOfPoints(); i++)
    {
        double p1[3], p2[3];
        points->GetPoint(i - 1, p1);
        points->GetPoint(i, p2);
        centerLineLength += sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
    }
    mitk::Surface* vessel = dynamic_cast<mitk::Surface*>(GetDataStorage()->GetNamedNode("tumorVessel")->GetData());
    pGuideWireMoulding->SetSurface(vessel->GetVtkPolyData());
    pGuideWireMoulding->SetBendFactor(1.0);

    WirePointsType wire;
    mitk::Point3D currentPoint;
    mitk::Vector3D direction;
    if (m_ui.InvertCB->isChecked())
    {
        pGuideWireMoulding->Simulate(ljlxPS->GetPoint(1).GetDataPointer(), (ljlxPS->GetPoint(3) - ljlxPS->GetPoint(1)).GetDataPointer(), centerLineLength*1.2, wire);
    }
    else
    {
        pGuideWireMoulding->Simulate(rkPS->GetPoint(0).GetDataPointer(), (rkPS->GetPoint(1) - rkPS->GetPoint(0)).GetDataPointer(), centerLineLength*1.2, wire);
    }

    auto wireAppendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
    auto wirePolyData = vtkSmartPointer<vtkPolyData>::New();
    auto pathPoints = vtkSmartPointer<vtkPoints>::New();
    auto wirePolyLine = vtkSmartPointer<vtkPolyLine>::New();
    auto wireCells = vtkSmartPointer<vtkCellArray>::New();
    auto collidedPoints = vtkSmartPointer<vtkPoints>::New();

    auto wireEnergy = vtkSmartPointer<vtkFloatArray>::New();
    wireEnergy->SetName("Curvature");//
    wireEnergy->SetNumberOfTuples(wire.size());


    if (m_ui.BendBox->isChecked())
    {
        pGuideWireMoulding->Moulde(m_ui.SpringRatioLE->text().toDouble(), m_ui.BendLengthLE->text().toDouble(), wire);
    }
    if (m_ui.SmoothCB->isChecked())
    {
        pGuideWireMoulding->Smooth(15.0, wire);
    }
    for (int i = 0; i < wire.size(); i++)
    {
        pathPoints->InsertNextPoint(wire[i].position);
        wirePolyLine->GetPointIds()->InsertNextId(i);
        wireEnergy->InsertNextTuple1(wire[i].dK);
        if (wire[i].isContact)
        {
            collidedPoints->InsertNextPoint(wire[i].position);
        }
        auto sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetCenter(wire[i].position);
        sphere->SetRadius(0.1);
        sphere->Update();
        wireAppendFilter->AddInputData(sphere->GetOutput());
    }
    wireCells->InsertNextCell(wirePolyLine);
    wirePolyData->SetPoints(pathPoints);
    wirePolyData->SetLines(wireCells);
    wirePolyData->GetPointData()->AddArray(wireEnergy);
    wirePolyData->GetPointData()->SetActiveScalars("Energy");


    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetName("Colors");//为该数组起名为"Colors"
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(wire.size());
    for (int i = 0; i < wire.size(); i++)
    {
        colors->InsertTuple3(i,
            255 * wire[i].isContact,
            0,
            255 * (1 - wire[i].isContact));
    }
    wirePolyData->GetPointData()->AddArray(colors);


    vtkSmartPointer<vtkTubeFilter> tubeFilter =
        vtkSmartPointer<vtkTubeFilter>::New();
    tubeFilter->SetInputData(wirePolyData);
    tubeFilter->SetRadius(0.1);
    tubeFilter->SetNumberOfSides(wire.size());
    tubeFilter->Update();
    wireAppendFilter->Update();
    mitk::DataNode::Pointer tubeNode = ImportVtkPolyData(tubeFilter->GetOutput(), "path");

    mitk::ColoredSurfaceVtkMapper::Pointer colorArrayMapper3D = mitk::ColoredSurfaceVtkMapper::New();
    tubeNode->SetMapper(mitk::BaseRenderer::Standard3D, colorArrayMapper3D);
    tubeNode->SetBoolProperty("scalar visibility", true);
    mitk::VtkScalarModeProperty::Pointer scalarModeProperty = mitk::VtkScalarModeProperty::New();
    scalarModeProperty->SetScalarModeToPointFieldData();
    tubeNode->SetProperty("scalar mode", scalarModeProperty);
    tubeNode->SetStringProperty("ColorArray", "Colors");
    tubeNode->SetDoubleProperty("ScalarsRangeMinimum", 0);
    tubeNode->SetDoubleProperty("ScalarsRangeMaximum", 180);
    tubeNode->Update();

    ImportVtkPolyData(wireAppendFilter->GetOutput(), "path point")->SetColor(1.0, 1.0, 0.0);

    RequestRenderWindowUpdate();
    pGuideWireMoulding->Release();
}

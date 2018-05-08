#include "SurfaceNormalView.h"
#include "iqf_main.h"
#include <vtkVersion.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPointData.h>
#include <vtkAppendPolyData.h>
#include <vtkLineSource.h>

#include "Rendering/SurfaceWithNormalsVtkMapper3D.h"

SurfaceNormalView::SurfaceNormalView() :MitkPluginView()
{

}

SurfaceNormalView::~SurfaceNormalView()
{

}

void SurfaceNormalView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreateSurfacePredicate());


    connect(m_ui.ApplyBtn, SIGNAL(clicked()), this, SLOT(Apply()));
}

void SurfaceNormalView::TestPointNormals(vtkPolyData* polydata)
{
    std::cout << "In TestPointNormals: " << polydata->GetNumberOfPoints() << std::endl;
    // Try to read normals directly
    bool hasPointNormals = GetPointNormals(polydata);

    if (!hasPointNormals)
    {
        std::cout << "No point normals were found. Computing normals..." << std::endl;

        // Generate normals
        vtkSmartPointer<vtkPolyDataNormals> normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
        normalGenerator->SetInputData(polydata);
        normalGenerator->ComputePointNormalsOn();
        normalGenerator->ComputeCellNormalsOn();
        normalGenerator->Update();
        /*
        // Optional settings
        normalGenerator->SetFeatureAngle(0.1);
        normalGenerator->SetSplitting(1);
        normalGenerator->SetConsistency(0);
        normalGenerator->SetAutoOrientNormals(0);
        normalGenerator->SetComputePointNormals(1);
        normalGenerator->SetComputeCellNormals(0);
        normalGenerator->SetFlipNormals(0);
        normalGenerator->SetNonManifoldTraversal(1);
        */

        polydata = normalGenerator->GetOutput();

        // Try to read normals again
        hasPointNormals = GetPointNormals(polydata);

        std::cout << "On the second try, has point normals? " << hasPointNormals << std::endl;
        if (hasPointNormals)
        {
            vtkFloatArray* normalDataFloat =
                vtkFloatArray::SafeDownCast(polydata->GetPointData()->GetArray("Normals"));
            int nc = normalDataFloat->GetNumberOfTuples();

            auto appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
            for (int i = 0; i < nc; i++)
            {
                mitk::Vector3D normal(normalDataFloat->GetTuple(i));
                normal.Normalize();
                mitk::Point3D center(polydata->GetPoint(i));
                mitk::Point3D target = center + normal * 1;

                vtkSmartPointer<vtkLineSource> lineSource =
                    vtkSmartPointer<vtkLineSource>::New();
                lineSource->SetPoint1(center.GetDataPointer());
                lineSource->SetPoint2(target.GetDataPointer());
                lineSource->Update();

                appendFilter->AddInputData(lineSource->GetOutput());
            }
            appendFilter->Update();
            mitk::DataNode::Pointer node = ImportVtkPolyData(appendFilter->GetOutput(), "point normal");
            node->SetColor(0.0, 1.0, 0.0);
            ImportVtkPolyData(polydata, "normal surface");
        }
    }
    else
    {
        std::cout << "Point normals were found!" << std::endl;
    }
}

void SurfaceNormalView::TestCellNormals(vtkPolyData* polydata)
{
    // Try to read normals directly
    bool hasCellNormals = GetCellNormals(polydata);

    if (!hasCellNormals)
    {
        std::cout << "No cell normals were found. Computing normals..." << std::endl;

        // Generate normals
        vtkSmartPointer<vtkPolyDataNormals> normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
        normalGenerator->SetInputData(polydata);
        normalGenerator->ComputePointNormalsOn();
        normalGenerator->ComputeCellNormalsOn();
        normalGenerator->Update();
        /*
        // Optional settings
        normalGenerator->SetFeatureAngle(0.1);
        normalGenerator->SetSplitting(1);
        normalGenerator->SetConsistency(0);
        normalGenerator->SetAutoOrientNormals(0);
        normalGenerator->SetComputePointNormals(1);
        normalGenerator->SetComputeCellNormals(0);
        normalGenerator->SetFlipNormals(0);
        normalGenerator->SetNonManifoldTraversal(1);
        */

        polydata = normalGenerator->GetOutput();

        // Try to read normals again
        hasCellNormals = GetCellNormals(polydata);
        if (hasCellNormals)
        {
            vtkFloatArray* normalDataFloat =
                vtkFloatArray::SafeDownCast(polydata->GetCellData()->GetArray("Normals"));
            int nc = normalDataFloat->GetNumberOfTuples();

            auto appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
            for (int i = 0; i < nc; i++)
            {
                mitk::Vector3D normal(normalDataFloat->GetTuple(i));
                normal.Normalize();
                double bounds[6];
                polydata->GetCellBounds(i, bounds);
                mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
                geometry->SetBounds(bounds);
                mitk::Point3D target = geometry->GetCenter() + normal * 1;

                vtkSmartPointer<vtkLineSource> lineSource =
                    vtkSmartPointer<vtkLineSource>::New();
                lineSource->SetPoint1(geometry->GetCenter().GetDataPointer());
                lineSource->SetPoint2(target.GetDataPointer());
                lineSource->Update();

                appendFilter->AddInputData(lineSource->GetOutput());
            }
            appendFilter->Update();
            mitk::DataNode::Pointer node = ImportVtkPolyData(appendFilter->GetOutput(), "cell normal");
            node->SetColor(0.0, 0.0, 1.0); ;
            ImportVtkPolyData(polydata, "normal surface");

        }

        std::cout << "On the second try, has cell normals? " << hasCellNormals << std::endl;

    }
    else
    {
        std::cout << "Cell normals were found!" << std::endl;
    }
}



bool SurfaceNormalView::GetPointNormals(vtkPolyData* polydata)
{
    std::cout << "In GetPointNormals: " << polydata->GetNumberOfPoints() << std::endl;
    std::cout << "Looking for point normals..." << std::endl;

    // Count points
    vtkIdType numPoints = polydata->GetNumberOfPoints();
    std::cout << "There are " << numPoints << " points." << std::endl;

    // Count triangles
    vtkIdType numPolys = polydata->GetNumberOfPolys();
    std::cout << "There are " << numPolys << " polys." << std::endl;

    ////////////////////////////////////////////////////////////////
    // Double normals in an array
    vtkDoubleArray* normalDataDouble =
        vtkDoubleArray::SafeDownCast(polydata->GetPointData()->GetArray("Normals"));

    if (normalDataDouble)
    {
        int nc = normalDataDouble->GetNumberOfTuples();
        std::cout << "There are " << nc
            << " components in normalDataDouble" << std::endl;
        return true;
    }

    ////////////////////////////////////////////////////////////////
    // float normals in an array
    vtkFloatArray* normalDataFloat =
        vtkFloatArray::SafeDownCast(polydata->GetPointData()->GetArray("Normals"));

    if (normalDataFloat)
    {
        int nc = normalDataFloat->GetNumberOfTuples();
        std::cout << "There are " << nc
            << " components in normalDataFloat" << std::endl;

        return true;
    }

    ////////////////////////////////////////////////////////////////
    // Point normals
    vtkDoubleArray* normalsDouble =
        vtkDoubleArray::SafeDownCast(polydata->GetPointData()->GetNormals());

    if (normalsDouble)
    {
        std::cout << "There are " << normalsDouble->GetNumberOfComponents()
            << " components in normalsDouble" << std::endl;
        return true;
    }

    ////////////////////////////////////////////////////////////////
    // Point normals
    vtkFloatArray* normalsFloat =
        vtkFloatArray::SafeDownCast(polydata->GetPointData()->GetNormals());

    if (normalsFloat)
    {
        std::cout << "There are " << normalsFloat->GetNumberOfComponents()
            << " components in normalsFloat" << std::endl;
        return true;
    }

    /////////////////////////////////////////////////////////////////////
    // Generic type point normals
    vtkDataArray* normalsGeneric = polydata->GetPointData()->GetNormals(); //works
    if (normalsGeneric)
    {
        std::cout << "There are " << normalsGeneric->GetNumberOfTuples()
            << " normals in normalsGeneric" << std::endl;

        double testDouble[3];
        normalsGeneric->GetTuple(0, testDouble);

        std::cout << "Double: " << testDouble[0] << " "
            << testDouble[1] << " " << testDouble[2] << std::endl;

        // Can't do this:
        /*
        float testFloat[3];
        normalsGeneric->GetTuple(0, testFloat);

        std::cout << "Float: " << testFloat[0] << " "
        << testFloat[1] << " " << testFloat[2] << std::endl;
        */
        return true;
    }


    // If the function has not yet quit, there were none of these types of normals
    std::cout << "Normals not found!" << std::endl;
    return false;

}


bool SurfaceNormalView::GetCellNormals(vtkPolyData* polydata)
{
    std::cout << "Looking for cell normals..." << std::endl;

    // Count points
    vtkIdType numCells = polydata->GetNumberOfCells();
    std::cout << "There are " << numCells << " cells." << std::endl;

    // Count triangles
    vtkIdType numPolys = polydata->GetNumberOfPolys();
    std::cout << "There are " << numPolys << " polys." << std::endl;

    ////////////////////////////////////////////////////////////////
    // Double normals in an array
    vtkDoubleArray* normalDataDouble =
        vtkDoubleArray::SafeDownCast(polydata->GetCellData()->GetArray("Normals"));

    if (normalDataDouble)
    {
        int nc = normalDataDouble->GetNumberOfTuples();
        std::cout << "There are " << nc
            << " components in normalDataDouble" << std::endl;
        return true;
    }

    ////////////////////////////////////////////////////////////////
    // Double normals in an array
    vtkFloatArray* normalDataFloat =
        vtkFloatArray::SafeDownCast(polydata->GetCellData()->GetArray("Normals"));

    if (normalDataFloat)
    {
        int nc = normalDataFloat->GetNumberOfTuples();
        std::cout << "There are " << nc
            << " components in normalDataFloat" << std::endl;
        return true;
    }

    ////////////////////////////////////////////////////////////////
    // Point normals
    vtkDoubleArray* normalsDouble =
        vtkDoubleArray::SafeDownCast(polydata->GetCellData()->GetNormals());

    if (normalsDouble)
    {
        std::cout << "There are " << normalsDouble->GetNumberOfComponents()
            << " components in normalsDouble" << std::endl;
        return true;
    }

    ////////////////////////////////////////////////////////////////
    // Point normals
    vtkFloatArray* normalsFloat =
        vtkFloatArray::SafeDownCast(polydata->GetCellData()->GetNormals());

    if (normalsFloat)
    {
        std::cout << "There are " << normalsFloat->GetNumberOfComponents()
            << " components in normalsFloat" << std::endl;
        return true;
    }

    /////////////////////////////////////////////////////////////////////
    // Generic type point normals
    vtkDataArray* normalsGeneric = polydata->GetCellData()->GetNormals(); //works
    if (normalsGeneric)
    {
        std::cout << "There are " << normalsGeneric->GetNumberOfTuples()
            << " normals in normalsGeneric" << std::endl;

        double testDouble[3];
        normalsGeneric->GetTuple(0, testDouble);

        std::cout << "Double: " << testDouble[0] << " "
            << testDouble[1] << " " << testDouble[2] << std::endl;

        // Can't do this:
        /*
        float testFloat[3];
        normalsGeneric->GetTuple(0, testFloat);

        std::cout << "Float: " << testFloat[0] << " "
        << testFloat[1] << " " << testFloat[2] << std::endl;
        */
        return true;
    }


    // If the function has not yet quit, there were none of these types of normals
    std::cout << "Normals not found!" << std::endl;
    return false;

}

void SurfaceNormalView::Apply()
{
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!surface)
    {
        return;
    }

    TestPointNormals(surface->GetVtkPolyData());

    TestCellNormals(surface->GetVtkPolyData());

}




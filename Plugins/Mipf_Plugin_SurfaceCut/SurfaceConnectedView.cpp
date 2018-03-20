#include "SurfaceConnectedView.h"

//vtk
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkDoubleArray.h>
#include "vtkXMLPolyDataWriter.h"

#include "mitkLookupTableProperty.h"

#include "Rendering/ColoredSurfaceVtkMapper.h"

#include "canvaspicker.h"
#include "scalepicker.h"

#include "MitkMain/IQF_MitkIO.h"

//qt
#include <QFileDialog>


SurfaceConnectedView::SurfaceConnectedView()  :m_NumberOfRegion(0)
{
}



SurfaceConnectedView::~SurfaceConnectedView()
{
}

void SurfaceConnectedView::CreateView()
{
    m_ui.setupUi(this);

    m_ui.SurfaceSelector->SetDataStorage(GetDataStorage());
    m_ui.SurfaceSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Surface>::New());
    connect(m_ui.ApplyBtn, SIGNAL(clicked()), this, SLOT(Apply()));
    connect(m_ui.SaveBtn, SIGNAL(clicked()), this, SLOT(Save()));
    connect(m_ui.ImportBtn, SIGNAL(clicked()), this, SLOT(Import()));

    
    m_picker = new CanvasPicker(m_ui.ColorEdit);

    /*ScalePicker *scalePicker = new ScalePicker(m_ui.ColorEdit);
    connect(scalePicker, SIGNAL(clicked(int, double)),
        m_ui.ColorEdit, SLOT(insertCurve(int, double)));*/

    connect(m_picker, SIGNAL(pointChanged(int, float)), this, SLOT(OpacityChanged(int, float)));


    connect(m_ui.ColorEdit, SIGNAL(colorChanged(const QColor&)), this, SLOT(ColorChanged(const QColor&)));
}

void SurfaceConnectedView::OpacityChanged(int regionID, float value)
{
    if (m_ui.SurfaceSelector->GetSelectedNode().IsNull())
    {
        return;
    }
    mitk::LookupTableProperty* lutp =  dynamic_cast<mitk::LookupTableProperty* >(m_ui.SurfaceSelector->GetSelectedNode()->GetProperty("lookup table"));
    if (lutp)
    {
        double color[4];
        lutp->GetLookupTable()->GetTableValue(regionID, color);
        color[3] = value / 255.0;
        lutp->GetLookupTable()->SetTableValue(regionID, color);
        RequestRenderWindowUpdate();
    }
}

void SurfaceConnectedView::ColorChanged(const QColor& c)
{
    m_picker->selectedPoint();
    if (m_ui.SurfaceSelector->GetSelectedNode().IsNull())
    {
        return;
    }
    mitk::LookupTableProperty* lutp = dynamic_cast<mitk::LookupTableProperty*>(m_ui.SurfaceSelector->GetSelectedNode()->GetProperty("lookup table"));
    if (lutp)
    {
        double rgba[4] = { c.redF(),c.greenF(),c.blueF(),1.0 };
        lutp->GetLookupTable()->SetTableValue(m_picker->selectedPoint(), rgba);
        RequestRenderWindowUpdate();
    }
}

void SurfaceConnectedView::Apply()
{
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_ui.SurfaceSelector->GetSelectedNode()->GetData());
    if (!surface)
    {
        return;
    }

    m_ui.SurfaceSelector->GetSelectedNode()->SetMapper(mitk::BaseRenderer::Standard3D, mitk::ColoredSurfaceVtkMapper::New());


    vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter =
        vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
    connectivityFilter->SetInputData(surface->GetVtkPolyData());
    connectivityFilter->SetExtractionModeToAllRegions();
    connectivityFilter->ColorRegionsOn();
    connectivityFilter->Update();
    m_NumberOfRegion = connectivityFilter->GetNumberOfExtractedRegions();


    vtkSmartPointer<vtkLookupTable> lut =
        vtkSmartPointer<vtkLookupTable>::New();
    lut->SetNumberOfTableValues(m_NumberOfRegion);
    lut->Build();
    for (int i = 0; i < m_NumberOfRegion; i++)
    {
        lut->SetTableValue(i, 1.0, 1.0, 1.0, 1.0);
    }
    mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
    mitkLut->SetVtkLookupTable(lut);
    mitk::LookupTableProperty::Pointer lutp = mitk::LookupTableProperty::New(mitkLut);
    m_ui.SurfaceSelector->GetSelectedNode()->SetProperty("lookup table", lutp);

    surface->SetVtkPolyData(connectivityFilter->GetOutput());

    m_ui.ColorEdit->RefreshNumberOfPoint(m_NumberOfRegion);
    RequestRenderWindowUpdate();

}

void SurfaceConnectedView::Import()
{

    QString name = QFileDialog::getOpenFileName(this, "Select one polydata with color to open",
        GetMitkReferenceInterface()->GetString("LastOpenFilePath"),
        "polydata (*.vtp *.vtk)");
    if (name.isEmpty())
        return ;
    GetMitkReferenceInterface()->SetString("LastOpenFilePath", QFileInfo(name).absolutePath().toStdString().c_str());

    IQF_MitkIO* pIO = (IQF_MitkIO*)GetInterfacePtr(QF_MitkMain_IO);
    mitk::DataNode* loadedNode = pIO->Load(name.toStdString().c_str());

    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(loadedNode->GetData());
    if (!surface)
    {
        return;
    }
    loadedNode->SetMapper(mitk::BaseRenderer::Standard3D, mitk::ColoredSurfaceVtkMapper::New());

    vtkDoubleArray* lookupTableArray = vtkDoubleArray::SafeDownCast(surface->GetVtkPolyData()->GetFieldData()->GetAbstractArray("lookup table"));
    if (!lookupTableArray)
    {
        return;
    }

    vtkSmartPointer<vtkLookupTable> lut =
        vtkSmartPointer<vtkLookupTable>::New();
    lut->SetNumberOfTableValues(lookupTableArray->GetNumberOfTuples());
    lut->Build();
    for (int i = 0; i < lookupTableArray->GetNumberOfTuples(); i++)
    {
        double* tuple = lookupTableArray->GetTuple(i);
        lut->SetTableValue(i, tuple);
    }
    mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
    mitkLut->SetVtkLookupTable(lut);
    mitk::LookupTableProperty::Pointer lutp = mitk::LookupTableProperty::New(mitkLut);
    loadedNode->SetProperty("lookup table", lutp);
}

void SurfaceConnectedView::Save()
{
    QString name = QFileDialog::getSaveFileName(
        NULL, "Save surface", GetMitkReferenceInterface()->GetString("LastFileSavePath"), "vtp (*.vtp)");
    if (name.isEmpty())
        return;
    GetMitkReferenceInterface()->SetString("LastFileSavePath", QFileInfo(name).absolutePath().toStdString().c_str());


    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_ui.SurfaceSelector->GetSelectedNode()->GetData());
    if (!surface)
    {
        return;
    }
    mitk::LookupTableProperty* lutp = dynamic_cast<mitk::LookupTableProperty*>(m_ui.SurfaceSelector->GetSelectedNode()->GetProperty("lookup table"));
    if (!lutp)
    {
        return;
    }

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->DeepCopy(surface->GetVtkPolyData());

    vtkSmartPointer<vtkDoubleArray> lookupTableArray =
        vtkSmartPointer<vtkDoubleArray>::New();

    vtkLookupTable* lut = lutp->GetLookupTable()->GetVtkLookupTable();
    for (int i=0;i<lut->GetNumberOfTableValues();i++)
    {
        double value[4] = { 1.0,1.0,1.0,1.0 };
        lut->GetTableValue(i, value);
        lookupTableArray->SetNumberOfComponents(4);
        lookupTableArray->SetName("lookup table");
        lookupTableArray->InsertNextTuple(value);
    }
    polyData->GetFieldData()->AddArray(lookupTableArray);

    auto writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    writer->SetInputData(polyData);
    writer->SetFileName(name.toStdString().c_str());
    writer->Write();

}

#include "TexturedVtkMapper3D.h"

//vtk
#include "vtkPolyData.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty2D.h"
#include "vtkLineSource.h"
#include "vtkPlaneSource.h"
#include <vtkSmartPointer.h>
#include <vtkTexture.h>
#include <vtkFloatArray.h>
#include <vtkPolygon.h>
#include <vtkPointData.h>
#include "vtkShader2.h"
#include "vtkShaderProgram2.h"
#include "vtkShader2Collection.h"
#include "vtkOpenGLProperty.h"
#include "vtkUniformVariables.h"
#include "vtkMath.h"
#include "vtkOpenGLHardwareSupport.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkImageBlend.h"
#include "vtkJPEGReader.h"

using namespace mitk;

TexturedVtkMapper3D::LocalStorage::LocalStorage()
{
    m_Actor = vtkSmartPointer<vtkActor>::New();
    m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();

    m_Actor->SetMapper(m_Mapper);
    m_Assembly->AddPart(m_Actor);

    m_Pgm = vtkSmartPointer<vtkShaderProgram2>::New();
    m_VertexShader = vtkSmartPointer<vtkShader2>::New();
    m_FragShader = vtkSmartPointer<vtkShader2>::New();
}

void TexturedVtkMapper3D::LocalStorage::InitShader(const std::string& vettexFilename, const std::string& fragmetFilename)
{
    if (vettexFilename.empty()&& fragmetFilename.empty())
    {
        return;
    }

    m_Pgm->GetShaders()->RemoveAllItems();
    std::ifstream vIn(vettexFilename, std::ios::in);
    if (vIn.is_open())
    {
        std::stringstream buffer;
        buffer << vIn.rdbuf();
        std::string vertexStr(buffer.str());
        m_VertexShader->SetType(VTK_SHADER_TYPE_VERTEX);
        m_VertexShader->SetSourceCode(vertexStr.c_str());
        m_Pgm->GetShaders()->AddItem(m_VertexShader);

    }

    std::ifstream fIn(fragmetFilename, std::ios::in);
    if (fIn.is_open())
    {
        std::stringstream buffer;
        buffer << fIn.rdbuf();
        std::string fragStr(buffer.str());
        m_FragShader->SetType(VTK_SHADER_TYPE_FRAGMENT);
        m_FragShader->SetSourceCode(fragStr.c_str());
        m_Pgm->GetShaders()->AddItem(m_FragShader);

    }

    vtkSmartPointer<vtkOpenGLProperty> openGLproperty =
        static_cast<vtkOpenGLProperty*>(m_Actor->GetProperty());
    openGLproperty->SetPropProgram(m_Pgm);
    openGLproperty->ShadingOn();
}

TexturedVtkMapper3D::TexturedVtkMapper3D():m_bTextureChanged(false), m_bShaderChanged(false)
{
}

TexturedVtkMapper3D::~TexturedVtkMapper3D()
{
}

void TexturedVtkMapper3D::SetTexture(vtkSmartPointer<vtkTexture> texture)
{
    m_texture = texture;
    m_bTextureChanged = true;
}

void TexturedVtkMapper3D::SetShaderSource(const std::string& vertexFilename, const std::string& fragmentFilename)
{
    m_vShaderFileName = vertexFilename;
    m_fShaderFileName = fragmentFilename;
    m_bShaderChanged = true;
}

const mitk::Surface *TexturedVtkMapper3D::GetInput() const
{
    return static_cast<Surface *>(GetDataNode()->GetData());
}

vtkProp* TexturedVtkMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    return ls->m_Assembly;
}

void TexturedVtkMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    ls->UpdateGenerateDataTime();

    if (m_bShaderChanged)
    {
        ls->InitShader(m_vShaderFileName, m_fShaderFileName);
        m_bShaderChanged = false;
    }

    ls->m_Pgm->SetContext(renderer->GetRenderWindow());
    ls->m_VertexShader->SetContext(renderer->GetRenderWindow());
    ls->m_FragShader->SetContext(renderer->GetRenderWindow());

    vtkPolyData* polydata = GetInput()->GetVtkPolyData();

    //vtkSmartPointer<vtkFloatArray> textureCoordinates =
    //    vtkSmartPointer<vtkFloatArray>::New();
    //textureCoordinates->SetNumberOfComponents(2);
    //textureCoordinates->SetName("TextureCoordinates");
    //float tuple[3] = { 0.0, 0.0 };
    //textureCoordinates->InsertNextTuple(tuple);
    //tuple[0] = 1.0; tuple[1] = 0.0;
    //textureCoordinates->InsertNextTuple(tuple);
    //tuple[0] = 1.0; tuple[1] = 1.0;
    //textureCoordinates->InsertNextTuple(tuple);
    //tuple[0] = 0.0; tuple[1] = 1.0;
    //textureCoordinates->InsertNextTuple(tuple);
    //polydata->GetPointData()->SetTCoords(textureCoordinates);

    ls->m_Mapper->SetInputData(polydata);


    if (m_bTextureChanged)
    {
        ls->m_Actor->SetTexture(m_texture);

        vtkSmartPointer<vtkUniformVariables> variables = vtkSmartPointer<vtkUniformVariables>::New();
        int textID[] = { 0 };
        variables->SetUniformi("texture", 1, textID);
        ls->m_Pgm->SetUniformVariables(variables);
        m_bTextureChanged = false;
    }   

    
}
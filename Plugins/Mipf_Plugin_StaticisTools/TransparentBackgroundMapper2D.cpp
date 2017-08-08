#include "TransparentBackgroundMapper2D.h"

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

#include <fstream>

using namespace mitk;


TransparentBackgroundMapper2D::LocalStorage::LocalStorage()
{
    m_QuadActor = vtkSmartPointer<vtkActor>::New();
    m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();

    m_QuadActor->SetMapper(m_Mapper);
    m_Assembly->AddPart(m_QuadActor);


    //enabel shader
    //const char* frag = "void propFuncFS(void){ gl_FragColor = vec4(255,0,0,1);}";
    std::ifstream test("S:/frag.txt" , std::ios::in);
    std::stringstream buffer;
    buffer << test.rdbuf();
    std::string frag(buffer.str());



    m_Pgm = vtkSmartPointer<vtkShaderProgram2>::New();

    m_Shader = vtkSmartPointer<vtkShader2>::New();
    m_Shader->SetType(VTK_SHADER_TYPE_FRAGMENT);
    m_Shader->SetSourceCode(frag.c_str());

    m_Pgm->GetShaders()->AddItem(m_Shader);

    vtkSmartPointer<vtkOpenGLProperty> openGLproperty =
        static_cast<vtkOpenGLProperty*>(m_QuadActor->GetProperty());
    openGLproperty->SetPropProgram(m_Pgm);
    openGLproperty->ShadingOn();

}


TransparentBackgroundMapper2D::TransparentBackgroundMapper2D()
{
}


TransparentBackgroundMapper2D::~TransparentBackgroundMapper2D()
{
}

const mitk::Image *TransparentBackgroundMapper2D::GetInput() const
{
    return static_cast<Image *>(GetDataNode()->GetData());
}


vtkProp* TransparentBackgroundMapper2D::GetVtkProp(mitk::BaseRenderer *renderer)
{
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    return ls->m_Assembly;
}

void TransparentBackgroundMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
    LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
    mitk::Image* image = dynamic_cast<mitk::Image*>(GetDataNode()->GetData());

    if (!image)
    {
        return;
    }
    mitk::Vector3D spacing = image->GetGeometry()->GetSpacing();
    const mitk::BaseGeometry::BoundingBoxType* box = image->GetGeometry()->GetBoundingBox();

    //create a textured quad
    vtkSmartPointer<vtkPoints> points =
        vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(box->GetMinimum()[0], box->GetMinimum()[1], box->GetMinimum()[2]);
    points->InsertNextPoint(box->GetMaximum()[0], box->GetMinimum()[1], box->GetMinimum()[2]);
    points->InsertNextPoint(box->GetMaximum()[0], box->GetMaximum()[1], box->GetMinimum()[2]);
    points->InsertNextPoint(box->GetMinimum()[0], box->GetMaximum()[1], box->GetMinimum()[2]);

    vtkSmartPointer<vtkCellArray> polygons =
        vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPolygon> polygon =
        vtkSmartPointer<vtkPolygon>::New();
    polygon->GetPointIds()->SetNumberOfIds(4); //make a quad
    polygon->GetPointIds()->SetId(0, 0);
    polygon->GetPointIds()->SetId(1, 1);
    polygon->GetPointIds()->SetId(2, 2);
    polygon->GetPointIds()->SetId(3, 3);

    polygons->InsertNextCell(polygon);

    vtkSmartPointer<vtkPolyData> quad =
        vtkSmartPointer<vtkPolyData>::New();
    quad->SetPoints(points);
    quad->SetPolys(polygons);

    vtkSmartPointer<vtkFloatArray> textureCoordinates =
        vtkSmartPointer<vtkFloatArray>::New();
    textureCoordinates->SetNumberOfComponents(2);
    textureCoordinates->SetName("TextureCoordinates");

    float tuple[3] = { 0.0, 0.0 };
    textureCoordinates->InsertNextTuple(tuple);
    tuple[0] = 1.0; tuple[1] = 0.0; 
    textureCoordinates->InsertNextTuple(tuple);
    tuple[0] = 1.0; tuple[1] = 1.0; 
    textureCoordinates->InsertNextTuple(tuple);
    tuple[0] = 0.0; tuple[1] = 1.0;
    textureCoordinates->InsertNextTuple(tuple);

    quad->GetPointData()->SetTCoords(textureCoordinates);

    // Apply the texture
    vtkSmartPointer<vtkTexture> texture =
        vtkSmartPointer<vtkTexture>::New();
    texture->SetInputData(image->GetVtkImageData());
    int dimen = image->GetDimension();

    ls->m_Mapper->SetInputData(quad);
    ls->m_QuadActor->SetTexture(texture);
    ls->m_Pgm->SetContext(renderer->GetRenderWindow());
    ls->m_Shader->SetContext(renderer->GetRenderWindow());
    
    float center[2], costheta[1], smallRadius[1], largeRadius[1],opacity[1],nodeColor[3];

    GetDataNode()->GetOpacity(opacity[0], renderer);
    GetDataNode()->GetColor(nodeColor, renderer);

    float imageWidth[] = { image->GetDimensions()[0] };
    float imageHeight[] = { image->GetDimensions()[1] };
    float theta_half = vtkMath::Pi() / 6.0;

    center[0] = imageWidth[0] / 2;
    largeRadius[0] = center[0] / sin(theta_half);
    center[1] = -(largeRadius[0] - imageHeight[0]);
    costheta[0] = cos(theta_half);
    smallRadius[0] = abs(center[1]) / costheta[0];


    //add uniform parameters
    vtkSmartPointer<vtkUniformVariables> variables = vtkSmartPointer<vtkUniformVariables>::New();
    variables->SetUniformf("origin", 2, center);
    variables->SetUniformf("costheta", 1, costheta);
    variables->SetUniformf("smallRadius", 1, smallRadius);
    variables->SetUniformf("largeRadius", 1, largeRadius);
    variables->SetUniformf("imageWidth", 1, imageWidth);
    variables->SetUniformf("imageHeight", 1, imageHeight);
    variables->SetUniformf("imageHeight", 1, imageHeight);
    variables->SetUniformf("opacity", 1, opacity);
    variables->SetUniformf("nodeColor", 3, nodeColor);
    ls->m_Pgm->SetUniformVariables(variables);

    //add texture
    int  textUInit = ls->m_Pgm->GetUniformLocation("quadTexture");;
    ls->m_Pgm->SetUniform1i(textUInit, 0);

    
    
}

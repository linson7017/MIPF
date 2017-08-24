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
#include "vtkOpenGLHardwareSupport.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkImageBlend.h"

#include "mitkImageVtkMapper2D.h"
#include "mitkWeakPointerProperty.h"
#include "vtkNeverTranslucentTexture.h"

#include "vtkMitkLevelWindowFilter.h"
#include "VTK_Helpers.h"

#include <fstream>

using namespace mitk;

const char fragStr[] = " "\
"uniform sampler2D quadTexture; \n"\
"uniform sampler2D fusionTexture; \n"\
"uniform float smallRadius;     \n"\
"uniform float largeRadius;  \n"\
"uniform float costheta;       \n"\
"uniform float imageWidth;  \n"\
"uniform float imageHeight;  \n"\
"uniform float opacity;     \n"\
"                       \n"\
"uniform vec3 nodeColor; \n"\
"uniform vec2 origin; \n"\
"              \n"\
"out vec4 color; \n"\
"                  \n"\
"void propFuncFS(void)  \n"\
"{                    \n"\
"    vec4 qColor = texture2D(quadTexture, gl_TexCoord[0].st);    \n"\
"    vec4 oColor = texture2D(fusionTexture, gl_TexCoord[0].st);    \n"\
"    vec2 tc = vec2(gl_TexCoord[0].s*imageWidth, (1 - gl_TexCoord[0].t)*imageHeight); \n"\
"                   \n"\
"   vec2 u = vec2(0, 1);   \n"\
"   vec2 p = tc - origin;    \n"\
"    float distance = length(p);  \n"\
"    vec2 np = normalize(p);     \n"\
"                                               \n"\
"    if (distance< smallRadius ||  \n"\
"        distance> largeRadius ||    \n"\
"        (u.r*np.r + u.g*np.g) < costheta)    \n"\
"    {                 \n"\
"        color = vec4(0.0, 0.0, 0.0, 0.0);   \n"\
"    }      \n"\
"   else    \n"\
"   {        \n"\
"      //  color = mix(oColor,qColor,opacity)\n"\
"        color = vec4(oColor.r*nodeColor.r, oColor.g*nodeColor.g, oColor.b*nodeColor.b, opacity);\n"\
"    }     \n"\
"} ";


TransparentBackgroundMapper2D::LocalStorage::LocalStorage()
{
    m_QuadActor = vtkSmartPointer<vtkActor>::New();
    m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_Assembly = vtkSmartPointer<vtkPropAssembly>::New();
   // m_FusionTexture = vtkSmartPointer<vtkTexture>::New();

    m_QuadActor->SetMapper(m_Mapper);
    m_Assembly->AddPart(m_QuadActor);


    //enabel shader
    //const char* frag = "void propFuncFS(void){ gl_FragColor = vec4(255,0,0,1);}";
  /*  std::ifstream test("S:/frag.txt" , std::ios::in);
    sd::stringstream buffer;
    buffer << test.rdbuf();
    std::string frag(buffer.str());*/

    m_Pgm = vtkSmartPointer<vtkShaderProgram2>::New();

    m_Shader = vtkSmartPointer<vtkShader2>::New();
    m_Shader->SetType(VTK_SHADER_TYPE_FRAGMENT);
    m_Shader->SetSourceCode(fragStr);

    m_Pgm->GetShaders()->AddItem(m_Shader);

    vtkSmartPointer<vtkOpenGLProperty> openGLproperty =
        static_cast<vtkOpenGLProperty*>(m_QuadActor->GetProperty());
    openGLproperty->SetPropProgram(m_Pgm);
    openGLproperty->ShadingOn();

    std::ofstream out("D:/program.txt");
    m_Pgm->PrintSelf(out, vtkIndent());

}


TransparentBackgroundMapper2D::TransparentBackgroundMapper2D() :m_fusionDataNode(NULL)
{
}

void TransparentBackgroundMapper2D::SetFusionRenderer(mitk::BaseRenderer* renderer)
{
    m_fusionRenderer = renderer;
}
void TransparentBackgroundMapper2D::SetFusionDataNode(mitk::DataNode* dataNode)
{
    m_fusionDataNode = dataNode;
}
void TransparentBackgroundMapper2D::SetFusion(bool fusion)
{
    m_bFusion = fusion;
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
    unsigned int* dimens = image->GetDimensions();
    mitk::Point3D origin = image->GetGeometry()->GetOrigin();   
    mitk::Vector3D nx = image->GetGeometry()->GetAxisVector(0);
    mitk::Vector3D ny = image->GetGeometry()->GetAxisVector(1);
    nx.Normalize();
    ny.Normalize();
    mitk::Point3D p1 = origin + nx*dimens[0]*spacing[0];
    mitk::Point3D p2 = origin + ny*dimens[1] * spacing[1];
    mitk::Point3D p12 = origin + (nx*dimens[0] * spacing[0]+ ny*dimens[1] * spacing[1]);

    //MITK_INFO << "Image point 0:" << origin;
    //MITK_INFO << "Image point 1:" << p1;
    //MITK_INFO << "Image point 2:" << p12;
    //MITK_INFO << "Image point 3:" << p2;

    //MITK_INFO << "Plane point 0:" << image->GetGeometry()->GetCornerPoint(1, 1, 1);
    //MITK_INFO << "Plane point 1:" << image->GetGeometry()->GetCornerPoint(0, 1, 1);
    //MITK_INFO << "Plane point 2:" << image->GetGeometry()->GetCornerPoint(0, 0, 1);
    //MITK_INFO << "Plane point 3:" << image->GetGeometry()->GetCornerPoint(1, 0, 1);


    //create a textured quad
    vtkSmartPointer<vtkPoints> points =
        vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(origin.GetDataPointer());
    points->InsertNextPoint(p1.GetDataPointer());
    points->InsertNextPoint(p12.GetDataPointer());
    points->InsertNextPoint(p2.GetDataPointer());

    

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

   /* mitk::ImageVtkMapper2D *imageMapper =
        dynamic_cast<mitk::ImageVtkMapper2D *>(m_fusionDataNode->GetMapper(1));
    vtkImageData* resliceData = imageMapper->GetLocalStorage(m_fusionRenderer)->m_ReslicedImage;
    if (m_bFusion&&resliceData)
    {
        mitk::Point3D forigin = m_fusionRenderer->GetCurrentWorldPlaneGeometry()->GetCornerPoint(1, 1, 1);
        mitk::Point3D fp1 = m_fusionRenderer->GetCurrentWorldPlaneGeometry()->GetCornerPoint(0, 1, 1);
        mitk::Point3D fp2 = m_fusionRenderer->GetCurrentWorldPlaneGeometry()->GetCornerPoint(0, 0, 1);
        mitk::Point3D fp12 = m_fusionRenderer->GetCurrentWorldPlaneGeometry()->GetCornerPoint(1, 0, 1);

    }*/

    float tuple[3] = { 0.0, 0.0 };
    textureCoordinates->InsertNextTuple(tuple);
    tuple[0] = 1.0; tuple[1] = 0.0; 
    textureCoordinates->InsertNextTuple(tuple);
    tuple[0] = 1.0; tuple[1] = 1.0; 
    textureCoordinates->InsertNextTuple(tuple);
    tuple[0] = 0.0; tuple[1] = 1.0;
    textureCoordinates->InsertNextTuple(tuple);
    quad->GetPointData()->SetTCoords(textureCoordinates);





   /* if (!m_fusionDataNode)
    {
        return;
    }*/
    // Apply the texture

    vtkSmartPointer<vtkNeverTranslucentTexture> texture =
        vtkSmartPointer<vtkNeverTranslucentTexture>::New();
    texture->SetInputData(image->GetVtkImageData());
    texture->Update();
    vtkSmartPointer<vtkNeverTranslucentTexture> texture2 =
        vtkSmartPointer<vtkNeverTranslucentTexture>::New();
    mitk::Image* resliceImage = dynamic_cast<mitk::Image*>(m_resliceDataNode->GetData());
    texture2->SetInputData(resliceImage->GetVtkImageData());
    texture2->Update();

    texture->SetBlendingMode(vtkTexture::VTK_TEXTURE_BLENDING_MODE_REPLACE);
    texture2->SetBlendingMode(vtkTexture::VTK_TEXTURE_BLENDING_MODE_ADD);


    ls->m_Mapper->SetInputData(quad);
    //ls->m_QuadActor->SetTexture(imageMapper->GetLocalStorage(m_fusionRenderer)->m_Texture);

    ls->m_Mapper->MapDataArrayToMultiTextureAttribute(
        vtkProperty::VTK_TEXTURE_UNIT_0, "TextureCoordinates",
        vtkDataObject::FIELD_ASSOCIATION_POINTS);
    ls->m_Mapper->MapDataArrayToMultiTextureAttribute(
        vtkProperty::VTK_TEXTURE_UNIT_1, "TextureCoordinates",
        vtkDataObject::FIELD_ASSOCIATION_POINTS);

    ls->m_QuadActor->GetProperty()->SetTexture(vtkProperty::VTK_TEXTURE_UNIT_0,
        texture);
    ls->m_QuadActor->GetProperty()->SetTexture(vtkProperty::VTK_TEXTURE_UNIT_1,
        texture2);




    ls->m_Pgm->SetContext(renderer->GetRenderWindow());
    ls->m_Shader->SetContext(renderer->GetRenderWindow());

    
    float center[2], costheta[1], smallRadius[1], largeRadius[1],opacity[1],nodeColor[3];

    GetDataNode()->GetOpacity(opacity[0], renderer);
    GetDataNode()->GetColor(nodeColor, renderer);

    float imageWidth[] = { image->GetDimensions()[0]*spacing[0] };
    float imageHeight[] = { image->GetDimensions()[1] * spacing[1] };
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
    variables->SetUniformf("opacity", 1, opacity);
    variables->SetUniformf("nodeColor", 3, nodeColor);
    variables->SetUniformf("spacing", 3, nodeColor);
    

    //add texture
 //   int  textUInit = ls->m_Pgm->GetUniformLocation("quadTexture");
    int textID[] = { 0 };
//    ls->m_Pgm->SetUniform1i(textUInit, textID);
   // int  fusionUInit = ls->m_Pgm->GetUniformLocation("fusionTexture");
    int fusionID[] = { 0 };
 //   ls->m_Pgm->SetUniform1i(fusionUInit, fusionID);

    variables->SetUniformi("quadTexture", 1, textID);
    variables->SetUniformi("fusionTexture", 1, fusionID);

    ls->m_Pgm->SetUniformVariables(variables);


}

#include "CurveProjectionMapper2D.h"

#include "mitkImage.h"
#include "mitkDataNode.h"
#include "GL/glew.h"

#include "vtkPolyData.h"

using namespace mitk;

GLuint vertShader;
GLuint fragShader;
GLuint programHandle;
GLuint texture;

CurveProjectionMapper2D::CurveProjectionMapper2D()
{
    int x = 0;
}


CurveProjectionMapper2D::~CurveProjectionMapper2D()
{
}

void CurveProjectionMapper2D::Paint(BaseRenderer *renderer)
{
    //bool visible = true;
    //GetDataNode()->GetVisibility(visible, renderer, "visible");

    //if (!visible)
   //     return;
    

    mitk::Image* image = dynamic_cast<mitk::Image*>(GetDataNode()->GetData());

    if (!image)
    {
        return;
    }

    mitk::Vector3D spacing = image->GetGeometry()->GetSpacing();
    const mitk::BaseGeometry::BoundingBoxType* box = image->GetGeometry()->GetBoundingBox();
    int la = 0;

    glDeleteTextures(1,&texture);
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); //��Ϊ�����˲��������ַ����־��
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, box->GetMaximum()[0], box->GetMaximum()[1],0,GL_RGBA,GL_UNSIGNED_BYTE, image->GetVtkImageData()->GetScalarPointer());


   // glColor4f(1.0, 0.0, 0.0,1.0);
    glBegin(GL_QUADS);
    glTexCoord2f(box->GetMinimum()[0], box->GetMinimum()[1]);
    glVertex2f(box->GetMinimum()[0], box->GetMinimum()[1]);
    glTexCoord2f(box->GetMaximum()[0], box->GetMinimum()[1]);
    glVertex2f(box->GetMaximum()[0], box->GetMinimum()[1]);
    glTexCoord2f(box->GetMaximum()[0], box->GetMaximum()[1]);
    glVertex2f(box->GetMaximum()[0], box->GetMaximum()[1]);
    glTexCoord2f(box->GetMinimum()[0], box->GetMaximum()[1]);
    glVertex2f(box->GetMinimum()[0], box->GetMaximum()[1]);
    glEnd();

}

void CurveProjectionMapper2D::InitGLSL()
{
    //��������shader
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    if (0 == vertShader)
    {
        fprintf(stderr, "Error creating vertex shader.\n");
        exit(1);
    }

    GLint result;

    //����ƬԪ��ɫ��
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (0 == fragShader)
    {
        fprintf(stderr, "ƬԪ��ɫ������ʧ��.\n");
        exit(1);
    }

    //c����shaderԴ��
    //const GLchar *shaderCode2 = loadShaderAsString("BrightNess.glsl");
    //glShaderSource(fragShader, 1, &shaderCode2, NULL);

    //delete[]shaderCode2;

    ////����shader
    //glCompileShader(fragShader);

    ////������״̬
    //glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
    //if (GL_FALSE == result)
    //{
    //    fprintf(stderr, "ƬԪ��ɫ������ʧ��!\n");
    //    GLint logLen;
    //    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLen);
    //    if (logLen > 0)
    //    {
    //        char * log = (char *)malloc(logLen);
    //        GLsizei written;
    //        glGetShaderInfoLog(fragShader, logLen, &written, log);
    //        fprintf(stderr, "Shaderlog:\n%s", log);
    //        free(log);
    //    }
    //}

    ////�����������
    //programHandle = glCreateProgram();
    //if (0 == programHandle)
    //{
    //    fprintf(stderr, "Error creating program object.\n");
    //    exit(1);
    //}

    ////����ɫ�����ӵ��������
    //glAttachShader(programHandle, fragShader);

    ////���ӳ���
    //glLinkProgram(programHandle);

    ////�������״̬
    //GLint status;
    //glGetProgramiv(programHandle, GL_LINK_STATUS, &status);
    //if (GL_FALSE == status)
    //{
    //    fprintf(stderr, "����ʧ��!\n");
    //    GLint logLen;
    //    glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH,
    //        &logLen);
    //    if (logLen > 0)
    //    {
    //        char * log = (char *)malloc(logLen);
    //        GLsizei written;
    //        glGetProgramInfoLog(programHandle, logLen,
    //            &written, log);
    //        fprintf(stderr, "Program log: \n%s", log);
    //        free(log);
    //    }

    //    glDeleteProgram(programHandle);
    //}

    //else
    //{
    //    glUseProgram(programHandle);
    //}
}

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
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); //设为线性滤波，避免字符出现锯齿
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
    //创建顶点shader
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    if (0 == vertShader)
    {
        fprintf(stderr, "Error creating vertex shader.\n");
        exit(1);
    }

    GLint result;

    //加载片元着色器
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (0 == fragShader)
    {
        fprintf(stderr, "片元着色器创建失败.\n");
        exit(1);
    }

    //c拷贝shader源码
    //const GLchar *shaderCode2 = loadShaderAsString("BrightNess.glsl");
    //glShaderSource(fragShader, 1, &shaderCode2, NULL);

    //delete[]shaderCode2;

    ////编译shader
    //glCompileShader(fragShader);

    ////检查编译状态
    //glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
    //if (GL_FALSE == result)
    //{
    //    fprintf(stderr, "片元着色器编译失败!\n");
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

    ////创建程序对象
    //programHandle = glCreateProgram();
    //if (0 == programHandle)
    //{
    //    fprintf(stderr, "Error creating program object.\n");
    //    exit(1);
    //}

    ////将着色器链接到程序对象
    //glAttachShader(programHandle, fragShader);

    ////链接程序
    //glLinkProgram(programHandle);

    ////检查链接状态
    //GLint status;
    //glGetProgramiv(programHandle, GL_LINK_STATUS, &status);
    //if (GL_FALSE == status)
    //{
    //    fprintf(stderr, "链接失败!\n");
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

#include "TexturedVtkMapper3D.h"

using namespace mitk;

TexturedVtkMapper3D::LocalStorage::LocalStorage()
{
}

TexturedVtkMapper3D::TexturedVtkMapper3D()
{
}

TexturedVtkMapper3D::~TexturedVtkMapper3D()
{
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
}
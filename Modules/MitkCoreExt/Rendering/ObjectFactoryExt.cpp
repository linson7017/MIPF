#include "ObjectFactoryExt.h"

#include "mitkSurface.h"
#include "mitkImage.h"

#include "mitkCoreObjectFactory.h"   

//mappers
#include "TexturedVtkMapper3D.h"
#include "NeedleVtkMapper2D.h"
#include "SegmentLineVtkMapper2D.h"
#include "CutUSImageVtkMapper.h"
#include "FreehandSurfaceCutMapper3D.h"
#include "ColoredSurfaceVtkMapper.h"

using namespace  mitk;

ObjectFactoryExt::ObjectFactoryExt()
{
}


ObjectFactoryExt::~ObjectFactoryExt()
{
}


mitk::Mapper::Pointer mitk::ObjectFactoryExt::CreateMapper(mitk::DataNode *node, MapperSlotId slotId)
{
    Mapper::Pointer mapper;

    if (dynamic_cast<Surface *>(node->GetData()) != nullptr)
    {
        if (slotId == BaseRenderer::Standard2D)
        {
            std::string mapperType;
            if (node->GetStringProperty("2d mapper type", mapperType))
            {
                if (mapperType.compare("needle") == 0)
                {
                    mapper = NeedleVtkMapper2D::New();
                }
                else if (mapperType.compare("segment line") == 0)
                {
                    mapper = SegmentLineVtkMapper2D::New();

                }
            }

        }
        else if (slotId == BaseRenderer::Standard3D)
        {
            std::string mapperType;
            if (node->GetStringProperty("3d mapper type", mapperType))
            {
                if (mapperType.compare("textured") == 0)
                {
                    mapper = TexturedVtkMapper3D::New();
                }
                else if (mapperType.compare("freehand surface cut") == 0)
                {
                    mapper = FreehandSurfaceCutMapper3D::New();
                }
                else if (mapperType.compare("colored") == 0)
                {
                    mapper = ColoredSurfaceVtkMapper::New();           
                }
            }
        }


        if (mapper.IsNotNull())
            mapper->SetDataNode(node);
    }
    else if (dynamic_cast<Image *>(node->GetData()) != nullptr)
    {
        std::string mapperType;
        if (node->GetStringProperty("mapper type", mapperType))
        {
            if (mapperType.compare("cut us image") == 0)
            {
                mapper = CutUSImageVtkMapper::New();
            }
        }
        if (mapper.IsNotNull())
            mapper->SetDataNode(node);
    }

    return mapper;
}


const char *mitk::ObjectFactoryExt::GetDescription() const
{
    return "Object Factory Ext";
}

const char *mitk::ObjectFactoryExt::GetFileExtensions()
{
    return nullptr;
}

mitk::CoreObjectFactoryBase::MultimapType mitk::ObjectFactoryExt::GetFileExtensionsMap()
{
    return MultimapType();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::ObjectFactoryExt::GetSaveFileExtensionsMap()
{
    return MultimapType();
}

const char *mitk::ObjectFactoryExt::GetSaveFileExtensions()
{
    return nullptr;
}

void mitk::ObjectFactoryExt::SetDefaultProperties(mitk::DataNode *node)
{
    if (node == nullptr)
        return;

    if (dynamic_cast<GeometryData *>(node->GetData()) != nullptr)
    {
    }
}

void RegisterObjectFactoryExt()
{
    static bool alreadyRegistered = false;

    if (!alreadyRegistered)
    {
        CoreObjectFactory::GetInstance()->RegisterExtraFactory(ObjectFactoryExt::New());
        alreadyRegistered = true;
    }
}


//struct RegisterObject
//{
//    RegisterObject()
//    {
//        RegisterObjectFactoryExt();
//    }
//};
//
//static RegisterObject registerObject;
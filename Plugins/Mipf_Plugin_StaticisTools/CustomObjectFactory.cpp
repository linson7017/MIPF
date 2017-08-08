#include "CustomObjectFactory.h"

#include "CurveProjectionVtkMapper2D.h"
#include "CurveProjectionMapper2D.h"
#include "TransparentBackgroundMapper2D.h"

#include "mitkImage.h"

#include "mitkCoreObjectFactory.h"

 using namespace  mitk;

CustomObjectFactory::CustomObjectFactory()
{
}


CustomObjectFactory::~CustomObjectFactory()
{
}


mitk::Mapper::Pointer mitk::CustomObjectFactory::CreateMapper(mitk::DataNode *node, MapperSlotId slotId)
{
    Mapper::Pointer mapper;

    if (dynamic_cast<Surface *>(node->GetData()) != nullptr)
    {
        if (slotId == BaseRenderer::Standard2D)
        {
            std::string mapperType;
            if (node->GetStringProperty("2d mapper type", mapperType))
            {
                if (mapperType.compare("curve projection")==0)
                {
                    mapper = CurveProjectionVtkMapper2D::New();
                }
            }          
        }
        else if (slotId == BaseRenderer::Standard3D)
        {
           // mapper = BoundingShapeVtkMapper3D::New();
        }

        if (mapper.IsNotNull())
            mapper->SetDataNode(node);
    }
    else if (dynamic_cast<Image *>(node->GetData()) != nullptr)
    {
        std::string mapperType;
        if (node->GetStringProperty("2d mapper type", mapperType))
        {
            if (mapperType.compare("shader rendering") == 0)
            {
                mapper = TransparentBackgroundMapper2D::New();
            }
        }
        if (mapper.IsNotNull())
            mapper->SetDataNode(node);
    }

    return mapper;
}


const char *mitk::CustomObjectFactory::GetDescription() const
{
    return "Custom Object Factory";
}

const char *mitk::CustomObjectFactory::GetFileExtensions()
{
    return nullptr;
}

mitk::CoreObjectFactoryBase::MultimapType mitk::CustomObjectFactory::GetFileExtensionsMap()
{
    return MultimapType();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::CustomObjectFactory::GetSaveFileExtensionsMap()
{
    return MultimapType();
}

const char *mitk::CustomObjectFactory::GetSaveFileExtensions()
{
    return nullptr;
}

void mitk::CustomObjectFactory::SetDefaultProperties(mitk::DataNode *node)
{
    if (node == nullptr)
        return;

    if (dynamic_cast<GeometryData *>(node->GetData()) != nullptr)
    {
        CurveProjectionVtkMapper2D::SetDefaultProperties(node);
        CurveProjectionVtkMapper2D::SetDefaultProperties(node);
    }
}

void RegisterCustomObjectFactory()
{
    static bool alreadyRegistered = false;

    if (!alreadyRegistered)
    {
        CoreObjectFactory::GetInstance()->RegisterExtraFactory(CustomObjectFactory::New());
        alreadyRegistered = true;
    }
}


struct RegisterCustomObject
{
    RegisterCustomObject()
    {
        RegisterCustomObjectFactory();
    }
};

static RegisterCustomObject registerCustomObject;
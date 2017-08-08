#ifndef CustomObjectFactory_h__
#define CustomObjectFactory_h__

#include "mitkCoreObjectFactoryBase.h"

#pragma once
namespace mitk 
{
    class CustomObjectFactory : public CoreObjectFactoryBase
    {
    public:
        mitkClassMacro(CustomObjectFactory, CoreObjectFactoryBase) itkFactorylessNewMacro(Self) itkCloneMacro(Self)

        virtual Mapper::Pointer CreateMapper(DataNode *node, MapperSlotId slotId) override;
        virtual void SetDefaultProperties(DataNode *node) override;
        virtual const char *GetFileExtensions() override;
        virtual CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
        virtual const char *GetSaveFileExtensions() override;
        virtual CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;
        virtual const char *GetDescription() const override;

    protected:
        CustomObjectFactory();
        ~CustomObjectFactory();
    };
}

void RegisterCustomObjectFactory();

#endif // CustomObjectFactory_h__

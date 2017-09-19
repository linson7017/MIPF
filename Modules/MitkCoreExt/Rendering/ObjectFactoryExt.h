#ifndef ObjectFactoryExt_h__
#define ObjectFactoryExt_h__

#pragma once
#include "mitkCoreObjectFactoryBase.h"
#include "qf_config.h"

namespace mitk
{
    class ObjectFactoryExt : public CoreObjectFactoryBase
    {
    public:
        mitkClassMacro(ObjectFactoryExt, CoreObjectFactoryBase) itkFactorylessNewMacro(Self) itkCloneMacro(Self)

        virtual Mapper::Pointer CreateMapper(DataNode *node, MapperSlotId slotId) override;
        virtual void SetDefaultProperties(DataNode *node) override;
        virtual const char *GetFileExtensions() override;
        virtual CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
        virtual const char *GetSaveFileExtensions() override;
        virtual CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;
        virtual const char *GetDescription() const override;

    protected:
        ObjectFactoryExt();
        ~ObjectFactoryExt();
    };
}

QF_API void RegisterObjectFactoryExt();

#endif // ObjectFactoryExt_h__


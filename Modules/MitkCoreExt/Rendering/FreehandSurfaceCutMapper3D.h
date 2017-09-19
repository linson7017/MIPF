#ifndef FreehandSurfaceCutMapper3D_h__
#define FreehandSurfaceCutMapper3D_h__

#pragma once

#include "mitkCommon.h"
#include "mitkVtkMapper.h"
#include "mitkSurface.h"

#include "vtkSmartPointer.h"
#include "qf_config.h"

class vtkActor;
class vtkPolyDataMapper;
class vtkPropAssembly;

namespace mitk
{

    class QF_API  FreehandSurfaceCutMapper3D : public VtkMapper
    {
    public:
        mitkClassMacro(FreehandSurfaceCutMapper3D, VtkMapper);
        itkFactorylessNewMacro(Self) itkCloneMacro(Self);
        virtual const mitk::Surface  *GetInput() const;
        /** \brief returns the a prop assembly */
        virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

        class LocalStorage : public mitk::Mapper::BaseLocalStorage
        {
        public:
            /* constructor */
            LocalStorage();

            /* destructor */
            ~LocalStorage() {}
            // actor
            vtkSmartPointer<vtkActor> m_Actor;
            vtkSmartPointer<vtkPolyDataMapper> m_Mapper;

            vtkSmartPointer<vtkPropAssembly> m_Assembly;
        };

        mitk::LocalStorageHandler<LocalStorage> m_LSH;
    protected:

        FreehandSurfaceCutMapper3D();
        ~FreehandSurfaceCutMapper3D();
        virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
    };

}

#endif // FreehandSurfaceCutMapper3D_h__

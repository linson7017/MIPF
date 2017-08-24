#ifndef TexturedVtkMapper3D_h__
#define TexturedVtkMapper3D_h__

#include "mitkCommon.h"
#include "mitkVtkMapper.h"
#include "mitkSurface.h"

#include "vtkSmartPointer.h"

class vtkActor2D;
class vtkPropAssembly;
class vtkFloatArray;
class vtkCellArray;
class vtkPolyDataMapper2D;

#pragma once

namespace mitk
{
    class MITKCORE_EXPORT TexturedVtkMapper3D : public VtkMapper
    {
    public:
        mitkClassMacro(TexturedVtkMapper3D, VtkMapper);
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

            vtkSmartPointer<vtkPropAssembly> m_Assembly;
        };

        mitk::LocalStorageHandler<LocalStorage> m_LSH;
    protected:

        TexturedVtkMapper3D();
        ~TexturedVtkMapper3D();

        virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
    };
}
#endif // TexturedVtkMapper3D_h__
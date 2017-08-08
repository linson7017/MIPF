#ifndef CurveProjectionVtkMapper2D_h__
#define CurveProjectionVtkMapper2D_h__

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
    class CurveProjectionVtkMapper2D : public VtkMapper
    {
    public:
        mitkClassMacro(CurveProjectionVtkMapper2D, VtkMapper);
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
            vtkSmartPointer<vtkActor2D> m_lineActor;
            vtkSmartPointer<vtkPolyDataMapper2D> m_Mapper;

            vtkSmartPointer<vtkPropAssembly> m_Assembly;
        };

        mitk::LocalStorageHandler<LocalStorage> m_LSH;
    protected:

        CurveProjectionVtkMapper2D();
        ~CurveProjectionVtkMapper2D();

        virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
    };
}



#endif // CurveProjectionMapper2D_h__

#ifndef SegmentLineVtkMapper2D_h__
#define SegmentLineVtkMapper2D_h__

#include "qf_config.h"

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
    class QF_API SegmentLineVtkMapper2D : public VtkMapper
    {
    public:
        mitkClassMacro(SegmentLineVtkMapper2D, VtkMapper);
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
            vtkSmartPointer<vtkPolyDataMapper2D> m_lineMapper;
            vtkSmartPointer<vtkPropAssembly> m_Assembly;
        };

        mitk::LocalStorageHandler<LocalStorage> m_LSH;
    protected:

        SegmentLineVtkMapper2D();
        ~SegmentLineVtkMapper2D();

        virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
    };
}



#endif // SegmentLineVtkMapper2D_h__

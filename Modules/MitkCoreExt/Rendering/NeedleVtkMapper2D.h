#ifndef NeedleVtkMapper2D_h__
#define NeedleVtkMapper2D_h__

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
    class QF_API NeedleVtkMapper2D : public VtkMapper
    {
    public:
        mitkClassMacro(NeedleVtkMapper2D, VtkMapper);
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
            vtkSmartPointer<vtkActor2D> m_lineBackActor;
            vtkSmartPointer<vtkActor2D> m_crossHairActor;
            vtkSmartPointer<vtkActor2D> m_intersectActor;
            vtkSmartPointer<vtkActor2D> m_extensionActor;

            vtkSmartPointer<vtkPolyDataMapper2D> m_lineMapper;
            vtkSmartPointer<vtkPolyDataMapper2D> m_lineBackMapper;
            vtkSmartPointer<vtkPolyDataMapper2D> m_crosshairMapper;
            vtkSmartPointer<vtkPolyDataMapper2D> m_intersectMapper;
            vtkSmartPointer<vtkPolyDataMapper2D> m_extensionMapper;


            vtkSmartPointer<vtkPropAssembly> m_Assembly;
        };

        mitk::LocalStorageHandler<LocalStorage> m_LSH;
    protected:

        NeedleVtkMapper2D();
        ~NeedleVtkMapper2D();

        virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
        void CreateCross2D(const mitk::Point2D& crossPosition,vtkPolyData* pOutput,double size=5.0);
        void CreateIntersect2D(const mitk::Point2D& intersectPosition, vtkPolyData* pOutput, double radius = 3.0);
        void CreateExtensionCord(const mitk::Point3D& startPosition,const mitk::Vector3D& direction,const mitk::Vector3D& normal,double length,vtkPolyData* pOutput);
    };
}



#endif // NeedleVtkMapper2D_h__

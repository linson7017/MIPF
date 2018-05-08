/********************************************************************
	FileName:    SurfaceWithNormalsVtkMapper3D.h
	Author:        Ling Song
	Date:           Month 3 ; Year 2018
	Purpose:	     
*********************************************************************/
#ifndef SurfaceWithNormalsVtkMapper3D_h__
#define SurfaceWithNormalsVtkMapper3D_h__

#include "qf_config.h"

#include "mitkCommon.h"
#include "mitkVtkMapper.h"
#include "mitkSurface.h"
#include "Rendering/ColoredSurfaceVtkMapper.h"

#include "vtkSmartPointer.h"

class vtkPolyDataMapper;
class vtkActor;
class vtkRenderer;
class vtkDataArray;

namespace mitk
{
    class QF_API SurfaceWithNormalsVtkMapper3D : public VtkMapper
    {
    public:
    public:
        mitkClassMacro(SurfaceWithNormalsVtkMapper3D, VtkMapper);
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

        SurfaceWithNormalsVtkMapper3D();
        ~SurfaceWithNormalsVtkMapper3D();

        virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
        static double ComputeScale(const double position[3], vtkRenderer *renderer);

        void GenerateArrows(vtkDataArray* normalData);
    };

}
#endif // SurfaceWithNormalsVtkMapper3D_h__
/********************************************************************
	FileName:    OrientationMarkerVtkMapper3D.h
	Author:        Ling Song
	Date:           Month 3 ; Year 2018
	Purpose:	     
*********************************************************************/
#ifndef OrientationMarkerVtkMapper3D_h__
#define OrientationMarkerVtkMapper3D_h__

#include "qf_config.h"

#include "mitkCommon.h"
#include "mitkVtkMapper.h"
#include "mitkSurface.h"
#include "Rendering/ColoredSurfaceVtkMapper.h"

#include "vtkSmartPointer.h"

class vtkPolyDataMapper;
class vtkActor;
class vtkRenderer;

namespace mitk
{
    class QF_API OrientationMarkerVtkMapper3D : public VtkMapper
    {
    public:
    public:
        mitkClassMacro(OrientationMarkerVtkMapper3D, VtkMapper);
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
            vtkSmartPointer<vtkActor> m_orientationActor;
            vtkSmartPointer<vtkPolyDataMapper> m_orientationMapper;
        };

        mitk::LocalStorageHandler<LocalStorage> m_LSH;
    protected:

        OrientationMarkerVtkMapper3D();
        ~OrientationMarkerVtkMapper3D();

        virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
        static double ComputeScale(const double position[3], vtkRenderer *renderer);
    };

}
#endif // OrientationMarkerVtkMapper3D_h__
/********************************************************************
	FileName:    ColoredSurfaceVtkMapper.h
	Author:        Ling Song
	Date:           Month 9 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef ColoredSurfaceVtkMapper_h__
#define ColoredSurfaceVtkMapper_h__

#include "mitkCommon.h"
#include "mitkSurfaceVtkMapper3D.h"
#include "mitkSurface.h"


#include "vtkSmartPointer.h"
#include "qf_config.h"

class vtkActor;
class vtkPolyDataMapper;
class vtkPropAssembly;

namespace mitk
{
    class QF_API  ColoredSurfaceVtkMapper : public SurfaceVtkMapper3D
    {
    public:
        mitkClassMacro(ColoredSurfaceVtkMapper, SurfaceVtkMapper3D);
        itkFactorylessNewMacro(Self) itkCloneMacro(Self);
      //  virtual const mitk::Surface  *GetInput() const;
        /** \brief returns the a prop assembly */
      //  virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

        //class LocalStorage : public mitk::Mapper::BaseLocalStorage
        //{
        //public:
        //    /* constructor */
        //    LocalStorage();

        //    /* destructor */
        //    ~LocalStorage() {}
        //    // actor
        //    vtkSmartPointer<vtkActor> m_Actor;
        //    vtkSmartPointer<vtkPolyDataMapper> m_Mapper;

        //    vtkSmartPointer<vtkPropAssembly> m_Assembly;
        //};

        //mitk::LocalStorageHandler<LocalStorage> m_LSH;
        virtual void ApplyAllProperties(mitk::BaseRenderer *renderer, vtkActor *actor);
    protected:
        ColoredSurfaceVtkMapper();
        ~ColoredSurfaceVtkMapper();
        //virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
       // virtual void ApplyProperties(mitk::BaseRenderer *renderer);
    };
}

#endif // ColoredSurfaceVtkMapper_h__
#ifndef TransparentBackgroundMapper2D_h__
#define TransparentBackgroundMapper2D_h__
#include "mitkCommon.h"
#include "mitkVtkMapper.h"
#include "mitkImage.h"

#include "vtkSmartPointer.h"

class vtkActor;
class vtkPropAssembly;
class vtkFloatArray;
class vtkCellArray;
class vtkPolyDataMapper;
class  vtkShaderProgram2;
class  vtkShader2;


namespace mitk
{
#pragma once
    class TransparentBackgroundMapper2D : public VtkMapper
    {
    public:
        mitkClassMacro(TransparentBackgroundMapper2D, VtkMapper);
        itkFactorylessNewMacro(Self) itkCloneMacro(Self);

        virtual const mitk::Image  *GetInput() const;
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
            vtkSmartPointer<vtkActor> m_QuadActor;
            vtkSmartPointer<vtkPolyDataMapper> m_Mapper;

            vtkSmartPointer<vtkPropAssembly> m_Assembly;

            vtkSmartPointer<vtkShaderProgram2> m_Pgm;

            vtkSmartPointer<vtkShader2> m_Shader;
        };

        mitk::LocalStorageHandler<LocalStorage> m_LSH;

    protected:
        virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;


        TransparentBackgroundMapper2D();
        ~TransparentBackgroundMapper2D();
    };
}

#endif // TransparentBackgroundMapper2D_h__

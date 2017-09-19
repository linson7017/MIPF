#ifndef TexturedVtkMapper3D_h__
#define TexturedVtkMapper3D_h__

#include "mitkCommon.h"
#include "mitkVtkMapper.h"
#include "mitkSurface.h"

#include "vtkSmartPointer.h"
#include "qf_config.h"

class vtkActor;
class vtkPropAssembly;
class vtkFloatArray;
class vtkCellArray;
class vtkPolyDataMapper;
class  vtkShaderProgram2;
class  vtkShader2;
class vtkTexture;

#pragma once

namespace mitk
{
    class QF_API TexturedVtkMapper3D : public VtkMapper
    {
    public:
        mitkClassMacro(TexturedVtkMapper3D, VtkMapper);
        itkFactorylessNewMacro(Self) itkCloneMacro(Self);

        virtual const mitk::Surface  *GetInput() const;
        /** \brief returns the a prop assembly */
        virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;
        void SetTexture(vtkSmartPointer<vtkTexture> texture);
        void SetShaderSource(const std::string& vertexFilename="", const std::string& fragmentFilename="");
        class LocalStorage : public mitk::Mapper::BaseLocalStorage
        {
        public:
            /* constructor */
            LocalStorage();

            /* destructor */
            ~LocalStorage() {}
            void InitShader(const std::string& vettexFilename,const std::string& fragmetFilename);

            // actor
            vtkSmartPointer<vtkActor> m_Actor;
            vtkSmartPointer<vtkPolyDataMapper> m_Mapper;

            vtkSmartPointer<vtkPropAssembly> m_Assembly;

            vtkSmartPointer<vtkShaderProgram2> m_Pgm;

            vtkSmartPointer<vtkShader2> m_VertexShader;
            vtkSmartPointer<vtkShader2> m_FragShader;
        };

        mitk::LocalStorageHandler<LocalStorage> m_LSH;
        vtkSmartPointer<vtkTexture> m_texture;
        bool m_bTextureChanged;

        std::string m_vShaderFileName;
        std::string m_fShaderFileName;
        bool m_bShaderChanged;
    protected:

        TexturedVtkMapper3D();
        ~TexturedVtkMapper3D();

        virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
    };
}
#endif // TexturedVtkMapper3D_h__
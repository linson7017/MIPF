#ifndef CutUSImageVtkMapper_h__
#define CutUSImageVtkMapper_h__

#include "qf_config.h"

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
    class QF_API CutUSImageVtkMapper : public VtkMapper
    {
    public:
        mitkClassMacro(CutUSImageVtkMapper, VtkMapper);
        itkFactorylessNewMacro(Self) itkCloneMacro(Self);

        virtual const mitk::Image  *GetInput() const;
        /** \brief returns the a prop assembly */
        virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

        void SetFusionRenderer(mitk::BaseRenderer* renderer);
        void SetFusionDataNode(mitk::DataNode* dataNode);
        void SetFusion(bool fusion);
        void SetResliceDataNode(mitk::DataNode* dataNode) { m_resliceDataNode = dataNode; }


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


        CutUSImageVtkMapper();
        ~CutUSImageVtkMapper();

        mitk::BaseRenderer* m_fusionRenderer;
        mitk::DataNode* m_fusionDataNode;
        mitk::DataNode* m_resliceDataNode;
        bool m_bFusion;
    };
}

#endif // CutUSImageVtkMapper_h__

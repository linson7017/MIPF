#ifndef WireVtkMapper_h__
#define WireVtkMapper_h__

#include "qf_config.h"

#include "mitkCommon.h"
#include "mitkVtkMapper.h"
#include "DataTypeExt/Wire.h"

#include "vtkSmartPointer.h"

class vtkActor;
class vtkPropAssembly;
class vtkFloatArray;
class vtkCellArray;
class vtkPolyDataMapper;


namespace mitk
{
    class QF_API WireVtkMapper : public VtkMapper
    {
    public:
        mitkClassMacro(WireVtkMapper, VtkMapper);
        itkFactorylessNewMacro(Self) itkCloneMacro(Self);

        virtual const mitk::Wire  *GetInput() const;
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
            vtkSmartPointer<vtkActor> m_lineActor;

            vtkSmartPointer<vtkPolyDataMapper> m_lineMapper;


            vtkSmartPointer<vtkPropAssembly> m_Assembly;
        };

        mitk::LocalStorageHandler<LocalStorage> m_LSH;
    protected:

        WireVtkMapper();
        ~WireVtkMapper();

        virtual void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;
      
    };
}



#endif // WireVtkMapper_h__

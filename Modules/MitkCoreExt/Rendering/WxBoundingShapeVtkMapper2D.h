/*===================================================================

* \author Guo Chu
* \date Jan,2018

===================================================================*/

#ifndef WxBoundingShapeVtkMapper2D_h
#define WxBoundingShapeVtkMapper2D_h

#include <mitkVtkMapper.h>
#include <vtkActor2D.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPropAssembly.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

#include "qf_config.h"

namespace mitk
{
  class QF_API WxBoundingShapeVtkMapper2D /*final*/ : public VtkMapper
  {
    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage();

      bool IsUpdateRequired(mitk::BaseRenderer *renderer, mitk::Mapper *mapper, mitk::DataNode *dataNode);

      vtkSmartPointer<vtkActor> m_Actor;
      vtkSmartPointer<vtkActor2D> m_HandleActor;
      vtkSmartPointer<vtkActor2D> m_SelectedHandleActor;
      vtkSmartPointer<vtkPolyDataMapper> m_Mapper;
      vtkSmartPointer<vtkPolyDataMapper2D> m_HandleMapper;
      vtkSmartPointer<vtkPolyDataMapper2D> m_SelectedHandleMapper;
      vtkSmartPointer<vtkCutter> m_Cutter;
      vtkSmartPointer<vtkPlane> m_CuttingPlane;
      unsigned int m_LastSliceNumber;
      std::vector<vtkSmartPointer<vtkSphereSource>> m_Handles;
      vtkSmartPointer<vtkPropAssembly> m_PropAssembly;
      double m_ZoomFactor;

    private:
      LocalStorage(const LocalStorage &);
      LocalStorage &operator=(const LocalStorage &);
    };

  public:
    static void SetDefaultProperties(DataNode *node, BaseRenderer *renderer = nullptr, bool overwrite = false);

    mitkClassMacro(WxBoundingShapeVtkMapper2D, VtkMapper);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      void ApplyColorAndOpacityProperties(BaseRenderer *, vtkActor *) override;
    vtkProp *GetVtkProp(BaseRenderer *renderer) override;

  private:
    WxBoundingShapeVtkMapper2D();
    ~WxBoundingShapeVtkMapper2D();

    WxBoundingShapeVtkMapper2D(const Self &);
    Self &operator=(const Self &);

    void GenerateDataForRenderer(BaseRenderer *renderer) override;

    class Impl;
    Impl *m_Impl;
  };
}

#endif
